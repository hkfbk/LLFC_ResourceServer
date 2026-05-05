#pragma once
#include "global.hpp"
#include <memory>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "msgnode.h"
#include <queue>
#include "boost/asio/strand.hpp"
#include <fstream>

namespace asio = boost::asio;
namespace ip = boost::asio::ip;
using boost::asio::ip::tcp;

class LogicSystem;
class CServer;
class CSession :public std::enable_shared_from_this<CSession>
{
	using uid_t = std::uint16_t;
	using msg_queue = std::queue<std::shared_ptr<SendNode>>;
	using head_msg_t = std::shared_ptr<MsgNode>;
	using recv_msg_t = std::shared_ptr<RecvNode>;
	using send_msg_t = std::shared_ptr<SendNode>;
public:
	CSession (asio::io_context& ioc_, CServer* p_server);
	~CSession ();
	[[nodiscard]] tcp::socket& get_socket () noexcept;
	[[nodiscard]] std::string get_session_id ()const noexcept;
	[[nodiscard]] uid_t get_uid () const noexcept;
	[[nodiscard]] bool is_stop () const noexcept;
	void start ();
	void close ();
	void send (ReqId id, const std::string& data);
	void write_handler (boost::system::error_code ec, std::shared_ptr<CSession> session);
	std::ofstream& get_file_stream ()noexcept;

private:
	void read_head ();
	void read_message_full (std::size_t read_len);
	tcp::socket m_sock;
	CServer* mp_server;
	const std::string m_session_id;
	uid_t m_uid;
	char m_data[BUFFER_TOTAL_LEN];
	// 消息队列
	msg_queue m_send_queue;
	head_msg_t m_head_node;
	recv_msg_t m_recv_node;
	// asio::strand队列
	asio::strand<asio::io_context::executor_type> m_strand;
	std::ofstream m_file_out;
	// 队列锁
	std::mutex m_send_lock;
	bool m_is_stop;
	bool m_is_head;// 检查是不是头部消息
};


struct LogicNode
	// 定义逻辑节点，以供逻辑系统使用
{
	std::shared_ptr<CSession> m_session;
	std::shared_ptr<RecvNode> m_recv_node;
};