#include <filesystem>
#include "singleton.hpp"
#include <memory>
#include <vector>
#include <atomic>
#include "fileworker.h"
class FileSystem :public Singleton<FileSystem>
{
	DEFINE_SINGLETON(FileSystem);
public:
	using FileTask = ::FileTask;
	using TaskPtr_t = std::shared_ptr<FileTask>;
	// 逻辑工作者数量
	static constexpr std::uint16_t s_logic_worker_count = 4;
	FileSystem();
	~FileSystem();
	void post_task_to_worker(TaskPtr_t task, std::size_t index);


private:
	std::vector<FileWorker> m_workers;
	
};
