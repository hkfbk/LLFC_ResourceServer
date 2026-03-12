#pragma once
#include "macro.hpp"
#include <thread>
#include <mutex>
#include <vector>
#include <memory>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>


#include "singleton.hpp"

namespace asio = boost::asio;

class AsioIoServicePool :public Singleton<AsioIoServicePool>
{
	friend class Singleton<AsioIoServicePool>;
	using work_t = asio::executor_work_guard<asio::io_context::executor_type>;
	using wrot_ptr = std::unique_ptr<work_t>;
	using ioservice = asio::io_context;
	__NOCOPY_MOVE__ (AsioIoServicePool);
	AsioIoServicePool (std::size_t size_ = 2 /*std::thread::hardware_concurrency() / 2*/);
public:
	~AsioIoServicePool ();
	ioservice& get_service ();
	void stop ();
private:
	std::vector<ioservice> m_services;
	std::vector<wrot_ptr> m_service_works;
	std::vector<std::thread> m_work_ths;
	std::size_t m_next_service;
	mutable std::mutex m_ioserver_lock;
	bool m_stop;
};


