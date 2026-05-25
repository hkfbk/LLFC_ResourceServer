#include "fileworker.h"
#include "global.hpp"
#include <filesystem>
#include "csession.h"

FileWorker::FileWorker()
	: m_b_stop(false)
{
	m_task_thr = std::thread([this] {
		while (m_b_stop)
		{
			try {
				std::unique_lock<std::mutex> l(m_mutex);
				m_cond.wait(l, [this] {
					return m_b_stop || !m_task_que.empty();
					});
				if (!m_task_que.empty())
				{
					auto task = std::move(m_task_que.front());
					m_task_que.pop();
					l.unlock(); // 尝试在执行任务之前开锁, 如果出现线程竞争则取消开锁
					task_callback(std::move(task));
				}
			} // !!!END try
			catch (std::exception e) {
				Assert(std::format("FileWorker::m_task_thr throw an exception:{}", e.what()));
			}
		}
		try {
			while (!m_task_que.empty()) // 在服务器停止工作后将数据剩余数据写入本地, 理论上此处不会发生竞争, 暂时不加锁
			{
				auto task = std::move(m_task_que.front());
				m_task_que.pop();
				task_callback(std::move(task));
			}
		} // !!!END try
		catch (std::exception e) {
			Assert(std::format("FileWorker::m_task_thr throw an exception:{}", e.what()));
		}
		});
}

void FileWorker::post_task(TaskPtr_t task, TaskType type)
{
	std::lock_guard l(m_mutex);
	bool is_empty = m_task_que.empty();
	m_task_que.push(std::move(task));
	if (is_empty) m_cond.notify_one();
}
void FileWorker::shutdown() noexcept
{
	std::lock_guard l(m_mutex);
	m_b_stop = true;
}
FileWorker::~FileWorker()
{
	m_task_thr.join();
}
void FileWorker::task_callback(TaskPtr_t task)
{
	try {
		// 创建路径
		auto&& opt = cfg["resource"].value<std::string>();
		if (!opt) Assert(false, std::format("config not found resource key"));
		auto&& basic_path = std::filesystem::path(*opt);
		auto&& path = basic_path / "static" / "bin";
		if (!fs::exists(path)) {
			spdlog::info(std::format("not found path:{}, creat path now.", path.string()));
			fs::create_directories(path.parent_path());
		}
		auto file_path = path / task->name;
		auto& out_file = task->session->get_file_stream();
		// 打开文件
		out_file.open(file_path);
		Assert(out_file.is_open(), std::format("file open failed:{}", task->name));
		// 计算文件写入的起始位置, 客户端目前没有这个功能待补充, 预定json字段 offset;
		out_file.seekp(task->offset);
		out_file << task->file_data;
		/// TODO:将文件写入本地
		out_file << task->file_data;
	}
	catch (std::exception e) {
		Assert(std::format("{}", e.what()));
	}
}