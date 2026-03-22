#include "logicsystem.h"
#include "boost/asio/write.hpp"
#include <nlohmann/json.hpp>
#include "global.hpp"

using namespace std::placeholders;
namespace json = nlohmann;
LogicSystem::LogicSystem ()
	: m_is_stop (false)
{
	register_callback ();
	m_word_thr = std::thread (&LogicSystem::deal_message, this);
}
LogicSystem::~LogicSystem ()
{
	m_is_stop = true;
	m_cond.notify_all ();
	spdlog::info ("wait locic_system_work_thr quit");
	m_word_thr.join ();
	spdlog::info (std::format ("LogicSystem destructer"));
}
void LogicSystem::register_callback ()
{
	m_callback_handler[ReqId::E_ID_TEST_MSG_REQ] = std::bind (&LogicSystem::test_send, this, _1, _2, _3);
	m_callback_handler[ReqId::E_ID_UPLOAD_FILE_REQ] = std::bind (&LogicSystem::do_upload_file_req, this, _1, _2, _3);
}

void LogicSystem::test_send (SessionPtr_t session, ReqId id, const std::string& data)
{
	spdlog::info (std::format ("test send msg:{}", data));
	(void) id;
	session->send (ReqId::E_ID_TEST_MSG_RSP, data);
}

void LogicSystem::do_upload_file_req (SessionPtr_t session, ReqId id, const std::string& data)
{
	try {
		json::json root = json::json::parse (data);
		json::json respond;
		Defer d ([&session, &respond] {
			session->send (ReqId::E_ID_UPLOAD_FILE_RSP, respond.dump ());
			});
		if (root.is_null () && !root.is_object ())
		{
			respond["sucess"] = enum_to_int (ResStaus::E_INVALID_JSON);
			respond["message"] = "json is invalid!!!";
			return;
		}
		std::string md5 = root["md5"];
		std::string filename = root["name"]; // 文件名字
		std::size_t seq = root["seq"]; // 第几次发送
		std::size_t trans_size = root["trans_size"];
		std::size_t total_size = root["total_size"];
		bool last = root["last"]; // 是不是最后一个包
		std::string base64_buffer = root["data"];
		std::optional path_pre = cfg["Path"]["ResourceRoot"].value<std::string> ();
		Assert (path_pre.has_value (), "resource root is null");

		fs::path p (path_pre.value ());
		spdlog::info (p.string ());
		spdlog::info (filename);
		spdlog::info (path_pre.value ());
		fs::path resource_path = p / "bin" / "static" / filename;
		if (!fs::exists (resource_path.parent_path ()))
		{
			spdlog::info (std::format ("not found path:{}, creat path now.", resource_path.parent_path ().string ()));
			fs::create_directories (resource_path.parent_path ());
		}
		//auto file_cur_size = fs::file_size (resource_path);
		std::ofstream& out = session->get_file_stream ();
		spdlog::info (std::format ("file stream open:{}", out.is_open ()));
		if (!out.is_open ())
		{
			if (seq == 1) out.open (resource_path, std::ios::binary | std::ios::trunc); // 第一个包，销毁文件
			else out.open (resource_path, std::ios::binary | std::ios::app); // 后续补充文件
		}
		Assert (out.is_open (), std::format ("resource file open failed, filename:{}", resource_path.string ()));
		std::string buffer = base64_decode (base64_buffer);
		spdlog::info (std::format ("wirte file:{} lenght:{}", resource_path.filename ().string (), buffer.size ()));
		out << buffer;
		if (last) // 最后一个包，关闭文件流
		{
			spdlog::info ("file stream close");
			out.close ();
		}
		respond["sucess"] = enum_to_int (ResStaus::E_SUCESS);
		respond["name"] = filename;
		respond["total_size"] = total_size;
		respond["trans_size"] = trans_size;
		respond["message"] = std::format ("success");
	}
	catch (std::filesystem::filesystem_error e)
	{
		Assert (false, std::format ("what:{}, error code:{}", e.what (), e.code ().message ()));
	}
	catch (std::runtime_error e)
	{
		Assert (false, e.what ());
	}
	catch (json::detail::parse_error e)
	{
		Assert (false, e.what ());
	}
	catch (std::exception e)
	{
		Assert (false, e.what ());
	}
}

void LogicSystem::post_message_to_logic_system (LogicNodePtr_t node_)
{
	{
		std::unique_lock l (m_mutex);
		bool was_empty = m_logic_que.empty ();
		m_logic_que.push (node_);
		if (was_empty) m_cond.notify_one ();
	}

	//// 确保队列为空后离开
	//std::unique_lock<std::mutex> l (m_mutex);
	//m_cond.wait (l, [this] {
	//	return m_logic_que.empty ();
	//	});
}

void LogicSystem::deal_message ()
{
	while (true)
	{
		// 检查逻辑队列，是否有未处理的消息或服务已关闭，如果没有或已关闭则通过条件变量等待
		std::unique_lock lock (m_mutex);
		//if (m_logic_que.empty () || m_is_stop) m_cond.wait (lock);
		m_cond.wait (lock, [this] {
			return !m_logic_que.empty () || m_is_stop;
			});
		if (!m_logic_que.empty ())
		{
			// 逐个处理逻辑信息
			auto&& msg_node = m_logic_que.front ();
			auto msg_id = msg_node->m_recv_node->get_msgID ();
			// 检查id是否存在
			if (!m_callback_handler.contains (msg_id))
			{
				m_logic_que.pop ();
				//if (m_logic_que.empty ()) m_cond.notify_one ();
				continue;
			}
			m_callback_handler[msg_id] (msg_node->m_session, msg_id, msg_node->m_recv_node->body);
			m_logic_que.pop ();
			//if (m_logic_que.empty ()) m_cond.notify_one ();
		}
		// 检查服务是否停止，如果停止则处理剩余信息后退出
		if (m_logic_que.empty () and m_is_stop)
			break;
	}
}