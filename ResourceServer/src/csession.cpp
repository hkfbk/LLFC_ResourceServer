#include "csession.h"
#include "cserver.h"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include <spdlog/spdlog.h>
#include "boost/asio/bind_executor.hpp"
#include "boost/asio/write.hpp"
#include "boost/asio/read.hpp"
#include "nlohmann/json.hpp"
#include "logicsystem.h"
namespace json = nlohmann;
using namespace std::placeholders;
CSession::CSession (asio::io_context& ioc_, CServer* p_server)
try
	: m_sock (ioc_)
	, mp_server (p_server)
	, m_session_id (std::move (boost::uuids::to_string (boost::uuids::random_generator ()())))
	, m_uid (0)
	, m_strand (asio::make_strand (ioc_))
	, m_is_stop (false)
	, m_is_head (true) // 默认为头部消息
	, m_data (0)
{
	m_head_node = std::make_shared<MsgNode> (HEAD_TOTAL_LEN);
}
	catch (std::exception e) {
		spdlog::error(std::format("CSession constructor failed:{}", e.what()));
}

tcp::socket& CSession::get_socket () noexcept
{
	return m_sock;
}
std::string CSession::get_session_id ()const noexcept
{
	return m_session_id;
}

auto CSession::get_uid () const noexcept->uid_t
{
	return m_uid;
}

bool CSession::is_stop () const noexcept
{
	return m_is_stop;
}

void CSession::start ()
{
	std::memset (m_data, 0, BUFFER_TOTAL_LEN);
	auto self = shared_from_this ();
	read_head ();
	return;
	//asio::async_read (m_sock, asio::buffer (m_data, BUFFER_TOTAL_LEN),
	m_sock.async_read_some (asio::buffer (m_data, BUFFER_TOTAL_LEN),
		[self] (boost::system::error_code ec, std::size_t bytes_transfered) {
			if (ec) { // 处理异常
				self->close ();
				spdlog::error (std::format ("callback failed:{}", ec.message ()));
				self->mp_server->clear_session (self->get_session_id ());
				return;
			}
			if (bytes_transfered == 0) return;
			std::size_t copy_len = 0; // 已经处理的长度
			while (bytes_transfered > 0) {
				// 处理头部
				if (self->m_is_head) {
					if (self->m_head_node->current_len + bytes_transfered < HEAD_TOTAL_LEN) {// 处理没有收全的头部信息
						std::memcpy (self->m_head_node->body + self->m_head_node->current_len, self->m_data + copy_len, bytes_transfered);
						self->m_head_node->current_len += std::uint32_t(bytes_transfered);
						// 继续监听
						self->start ();
						return;
					}
					// 计算剩余长度
					auto missing_len = self->m_head_node->total_len - self->m_head_node->current_len;
					if (missing_len == 0) { // 头部已满
						self->m_is_head = false;
						continue;
					}
					std::memcpy (self->m_head_node->body + self->m_head_node->current_len, self->m_data + copy_len, missing_len);
					self->m_head_node->current_len += missing_len;
					bytes_transfered -= missing_len;
					copy_len += missing_len;
					self->m_is_head = false;
					// 解构数据
					MsgId_t id = 0;
					std::memcpy (&id, self->m_head_node->body, HEAD_ID_LEN); // 先取出消息id
					id = ops::network_to_host_short (id); // 转为本地字节序
					HeadData_t msg_len = 0;
					std::memcpy (&msg_len, self->m_head_node->body + HEAD_ID_LEN, HEAD_DATA_LEN);// 取出消息体长度
					msg_len = ops::network_to_host_long (msg_len);
					if (msg_len > MSG_TOTAL_LEN) { // 验证消息长度
						spdlog::error (std::format ("messgen buffer overflow, total lenght:{}, reveive lenght:{}", MSG_TOTAL_LEN, msg_len));
						// 关闭会话
						self->close ();
						self->mp_server->clear_session (self->get_session_id ());
						return;
					}
					// 构造接收节点
					self->m_recv_node = std::make_shared<RecvNode> (msg_len, static_cast<ReqId>(id));
					self->m_head_node->clear (); // 信息已经用完了，先清除出去
					if (bytes_transfered <= 0) { // 正好只有一个头部的长度, 继续接收信息
						self->start ();
						return;
					}
					continue; // 还有剩余数据，继续解析数据


				}

				// 头部处理完成，但是还有数据没有处理
				// 检查接收节点还有多少剩余
				MsgLen_t remaining_len = self->m_recv_node->total_len - self->m_recv_node->current_len;
				if (remaining_len > bytes_transfered) {//消息不够， 拷贝下来
					std::memcpy (self->m_recv_node->body + self->m_recv_node->current_len, self->m_data + copy_len, bytes_transfered);
					self->m_recv_node->current_len += std::uint32_t(bytes_transfered);
					// 消息全部取出，继续监听其它数据
					self->start ();
					return;
				}
				// 处理消息接收够的情况
				// 取出消息体长度
				std::memcpy (self->m_recv_node->body + self->m_recv_node->current_len, self->m_data + copy_len, remaining_len);
				self->m_recv_node->body[self->m_recv_node->total_len] = '\0';
				copy_len += remaining_len;
				bytes_transfered -= remaining_len;
				self->m_recv_node->current_len += remaining_len;
				// 消息收取完毕，处理消息
				// 先打印出来
				//spdlog::info (self->m_recv_node->current_len);
				//spdlog::info (self->m_recv_node->body);
				auto hash_id = self->m_hash(self->get_session_id()) % LogicSystem::s_logic_pool_size;
				LogicSystem::get_instance ().post_message_to_logic_system (std::make_shared<LogicNode> (self, self->m_recv_node), hash_id);
				self->m_is_head = true;
				if (bytes_transfered == 0) { // 所有消息均已处理完毕
					self->start ();
					return;
				}
			}


		});

}

void CSession::read_head ()
{
	std::memset (m_data, 0, BUFFER_TOTAL_LEN);
	m_head_node->clear ();
	asio::async_read (m_sock, asio::buffer (m_data, HEAD_TOTAL_LEN),
		[self = shared_from_this ()] (boost::system::error_code ec, std::size_t bytes_transfered) {
			if (ec) { // 处理异常
				self->close ();
				spdlog::error (std::format ("callback failed:{}", ec.message ()));
				self->mp_server->clear_session (self->get_session_id ());
				return;
			}
			if (bytes_transfered == 0) return;
			// 处理头部信息
			std::memcpy (self->m_head_node->body, self->m_data, HEAD_TOTAL_LEN);
			self->m_head_node->current_len += HEAD_TOTAL_LEN;
			// 将id从节点取出
			std::int16_t id = 0;
			std::memcpy (&id, self->m_head_node->body, HEAD_ID_LEN);
			id = ops::network_to_host_short (id);
			// 将长度从头部取出
			std::int32_t body_len = 0;
			std::memcpy (&body_len, self->m_head_node->body + HEAD_ID_LEN, HEAD_DATA_LEN);
			spdlog::info (std::format ("body_len:{}", body_len));
			body_len = ops::network_to_host_long (body_len);
			spdlog::info (std::format ("body_len:{}", body_len));
			// 构造消息节点
			self->m_recv_node = std::make_shared<RecvNode> (body_len, static_cast<ReqId>(id));
			spdlog::info (std::format ("head receive len:{}", bytes_transfered));
			// 调用接收消息体的函数
			self->read_message_full (body_len);

		});

}
void CSession::read_message_full (std::size_t read_total_len)
{
	std::memset (m_data, 0, BUFFER_TOTAL_LEN);
	asio::async_read (m_sock, asio::buffer (m_data, std::min<std::size_t> (read_total_len, BUFFER_TOTAL_LEN)),
		[self = shared_from_this (), read_total_len] (boost::system::error_code ec, std::size_t bytes_transfered) {
			if (ec) { // 处理异常
				self->close ();
				spdlog::error (std::format ("callback failed:{}", ec.message ()));
				self->mp_server->clear_session (self->get_session_id ());
				return;
			}
			Assert (bytes_transfered > 0, std::format ("session:{} | read 0 lenght message!!!", self->get_session_id ()));

			// 处理头部消息
			// 先将已有消息拷贝到缓冲区
			std::memcpy (self->m_recv_node->body + self->m_recv_node->current_len, self->m_data, bytes_transfered);
			// 记录长度
			self->m_recv_node->current_len += std::uint32_t(bytes_transfered);
			//判断是否收全
			if (self->m_recv_node->is_fulled ()) // 收全了, 投递消息并开启新的消息监听
			{
				spdlog::info (std::format ("data len:{}, total len:{}", self->m_recv_node->current_len, self->m_recv_node->total_len));
				
				LogicSystem::get_instance ().post_message_to_logic_system (std::make_shared<LogicNode> (self, self->m_recv_node), self->m_hash(self->get_session_id()) % LogicSystem::s_logic_pool_size);

				self->read_head ();
				return;
			}
			else // 未收全, 计算剩余大小并监听
			{
				std::int64_t residual = static_cast<std::int64_t>(read_total_len - bytes_transfered);
				Assert (residual > 0, "residual lenght is too small!!!");
				self->read_message_full (residual);
				return;
			}
		}
	);
}

void CSession::send (ReqId id, const std::string& data)
{
	std::unique_lock l (m_send_lock);
	auto que_size = m_send_queue.size ();
	if (que_size > MAX_SENDQUE)
	{
		spdlog::error (std::format ("Session id:{}, send sequens lenght to loog:{}", m_session_id, que_size));
		return;
	}
	m_send_queue.push (std::make_shared<SendNode> (data.c_str (), data.length (), id));
	if (que_size > 0) return; // 已经有线程在处理了，先退出
	auto& msg_node = m_send_queue.front ();
	spdlog::info (std::format ("Send message:{}, id:{}, lenght:{}, date len:{}", msg_node->body + HEAD_TOTAL_LEN, enum_to_int (msg_node->msg_id), msg_node->current_len, data.length ()));
	asio::async_write (m_sock, asio::buffer (msg_node->body, msg_node->current_len),
		std::bind (&CSession::write_handler, this, _1, shared_from_this ()));

}

void CSession::write_handler (boost::system::error_code ec, std::shared_ptr<CSession> session)
{
	if (ec)
	{
		spdlog::error (std::format ("wirte_handler, session id:{}, error message:{}", m_session_id, ec.message ()));
		close ();
		mp_server->clear_session (m_session_id);
		return;
	}
	m_send_queue.pop ();
	if (m_send_queue.empty ())
		return;
	auto&& msg_node = m_send_queue.front ();
	asio::async_write (m_sock, asio::buffer (msg_node->body, msg_node->current_len),
		std::bind (&CSession::write_handler, this, _1, shared_from_this ()));
}

std::ofstream& CSession::get_file_stream ()noexcept
{
	return m_file_out;
}
void CSession::close ()
{
	m_sock.close ();
	m_is_stop = true;
}

CSession::~CSession ()
{
	spdlog::info (std::format ("CSession destructer, id:{}", m_session_id));
}
