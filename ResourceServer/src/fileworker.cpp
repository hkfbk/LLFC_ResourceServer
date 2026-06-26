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
					TaskPtr_t task = std::move(m_task_que.front());
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
				auto&& task = std::move(m_task_que.front());
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
	if (is_empty || true) m_cond.notify_one();
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
		spdlog::info (__func__);
		//auto&& opt = cfg["Path"]["ResourceRoot"].value<std::string> ();
		//Assert(opt.has_value(), std::format ("config not found ResourceRoot key"));
		//auto&& basic_path = std::filesystem::path(opt.value_or("defaut"));
		//auto&& path = basic_path / "static" / "bin";
		//if (!fs::exists(path)) {
		//	spdlog::info(std::format("not found path:{}, creat path now.", path.string()));
		//	fs::create_directories(path.parent_path());
		//}
		auto& file_path = task->path;
		auto& out_file = task->session->get_file_stream();
		Assert (!file_path.has_filename () && !fs::exists (file_path), std::format ("is not file path"));
		//Assert(!file_path.has_filename () && !fs::exists (file_path),std::format ("{} is not file path", file_path.native()));

		// 打开文件
		//if (task->seq == 1) out_file.open (file_path, std::ios::binary | std::ios::trunc); // 第一个包，销毁文件
		//else out_file.open (file_path, std::ios::binary | std::ios::app); // 后续补充文件
		out_file.open (file_path, std::ios::binary);
		//out_file.open(file_path);
		Assert(out_file.is_open(), std::format("file open failed:{}, seq:{}", task->name, task->seq));
		// 计算文件写入的起始位置, 客户端目前没有这个功能待补充, 预定json字段 offset;
		INFOLOG (std::format("file:{}, write offset:{}", task->name, task->offset));
		out_file.seekp(task->offset);
		/// TODO:将文件写入本地
		out_file << task->file_data;
	}
	catch (std::exception e) {
		Assert(std::format("{}", e.what()));
	}
}