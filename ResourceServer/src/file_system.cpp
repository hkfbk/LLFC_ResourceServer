#include "file_system.h"
#include "fileworker.h"
#include "global.hpp"

FileSystem::FileSystem()
{
	m_workers.reserve(s_logic_worker_count);
	for (std::size_t idx = 0; idx < s_logic_worker_count;idx++)
		m_workers.emplace_back(std::make_unique<FileWorker>());
}

FileSystem::~FileSystem()
{
	for (auto&& worker : m_workers)
		worker->shutdown();
}

void FileSystem::post_task_to_worker(TaskPtr_t task, std::size_t index)
{
	try {
		m_workers.at (index)->post_task (std::move (task));
	}
	catch (std::out_of_range e) {
		Assert (e.what ());
	}
}





