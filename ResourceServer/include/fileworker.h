#pragma once
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>

class CSession;
struct FileTask // 文件任务的信息
{
	std::shared_ptr<CSession> session;
	std::size_t sep;
	std::size_t total_size;
	std::size_t trans_size;
	std::string name;
	std::string file_data;
	std::size_t offset; // 文件指针开始写入的位置
	bool last;
};

class FileWorker
{
public:
	enum class TaskType :std::uint16_t {
		E_UPLOAD, // 上传
		E_DOWNLOAD, // 下载
	};
	using TaskPtr_t = std::unique_ptr<FileTask>;
	using enum TaskType;
	FileWorker();
	~FileWorker();
	void post_task(TaskPtr_t task, TaskType type = E_UPLOAD);
	void shutdown() noexcept;
private:
	void task_callback(TaskPtr_t task);
	std::queue<TaskPtr_t> m_task_que;
	std::thread m_task_thr;
	std::mutex m_mutex;
	std::condition_variable m_cond;
	bool m_b_stop;
};