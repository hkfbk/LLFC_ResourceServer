#pragma once
#include <memory>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address.hpp>
#include <map>

namespace asio = boost::asio;
namespace ip = boost::asio::ip;
using tcp = boost::asio::ip::tcp;
class CSession;
class CServer :public std::enable_shared_from_this<CServer>
{
public:
	CServer (asio::io_context& ioc_, unsigned short port_);
	~CServer ();
	void start ();
	void clear_session (const std::string& session_id);
private:
	asio::io_context& m_ioc;
	tcp::acceptor m_accept;
	std::map<std::string, std::shared_ptr<CSession>> m_sessions;
};
