#include "ioservicepool.h"
#include <format>
#include <spdlog/spdlog.h>

AsioIoServicePool::AsioIoServicePool (std::size_t size_)
	: m_services (size_)
	, m_service_works (size_)
	, m_next_service (0)
	, m_stop (false)
{
	// 为线程池中的service绑定work，防止在没有被使用时停止
	for (int i = 0; i < size_; i++)
		m_service_works[i] = std::make_unique<work_t> (asio::make_work_guard (m_services[i]));
	for (int i = 0; i < size_; i++)
		m_work_ths.emplace_back ([this, i] {
		m_services[i].run ();
			});


	// 在线程中启动服务
	for (auto& service : m_services)
		m_work_ths.emplace_back ([&service] {
		service.run ();
			});
}

auto AsioIoServicePool::get_service ()
->ioservice&
{
	std::lock_guard l (m_ioserver_lock);
	auto& service = m_services[m_next_service++];
	if (m_next_service == m_services.size ())
		m_next_service = 0;
	return service;
}

void AsioIoServicePool::stop ()
{
	if (m_stop) return;
	// 释放ioservice中绑定的work, 在任务全部完成后自动释放
	spdlog::info (std::format ("AsioIoServicePool stop"));
	for (auto&& work : m_service_works)
		work.reset ();
	for (auto&& service : m_services)
		service.stop ();
	// 阻塞等待各个线程结束
	for (auto&& th : m_work_ths)
		th.join ();
	m_stop = true;
}

AsioIoServicePool::~AsioIoServicePool ()
{
	if (m_stop) return;
	stop ();
	spdlog::info (std::format ("AsioIoServicePool destructor"));
}