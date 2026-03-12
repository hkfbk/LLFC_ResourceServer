#include "cserver.h"
#include "csession.h"
#include <iostream>
#include <format>
#include <spdlog/spdlog.h>
#include "ioservicepool.h"
#include "boost/system/error_code.hpp"

CServer::CServer (asio::io_context& ioc_, unsigned short port_)
	: m_ioc (ioc_)
	//, m_accept (ioc_, port_) // 这个构造函数不会开启连接
	, m_accept (ioc_, tcp::endpoint (tcp::v4 (), port_))

{ }

CServer::~CServer ()
{
	spdlog::debug (std::format ("{}\n", __FUNCTION__));
}

void CServer::start ()
{
	spdlog::info (std::format ("CServer::start"));
	asio::io_context& ioc = AsioIoServicePool::get_instance ().get_service ();
	auto&& self = shared_from_this ();
	auto session = std::make_shared<CSession> (ioc, this);
	m_accept.async_accept (session->get_socket (), [self, session] (boost::system::error_code ec) {
		if (ec) {
			spdlog::error (std::format ("connected failed:{}", ec.message ()));
			self->start ();
			return;
		}
		session->start ();
		// 连接成功，保存session
		self->m_sessions[session->get_session_id ()] = session;

		//监听一次新连接
		self->start ();
		});
}


void CServer::clear_session (const std::string& session_id)
{
	m_sessions.erase (session_id);
}