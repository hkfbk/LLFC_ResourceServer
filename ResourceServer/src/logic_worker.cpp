#include "logic_worker.h"
#include <nlohmann/json.hpp>
#include "csession.h"
#include "file_system.h"
#include "global.hpp"
#include <filesystem>
#include "macro.hpp"
using namespace std::placeholders;
namespace json = nlohmann;
namespace fs = std::filesystem;
//LogicNode::LogicNode (std::shared_ptr<CSession> session, std::unique_ptr<RecvNode>&& recv_node)
//	:m_session (session), m_recv_node (std::move (recv_node))
//
//{ }

LogicWork::LogicWork()
	: m_b_stop(false)
{
	try {

	for (int i = 0; i < s_work_logic_count; i++)
	m_work_thr[i] = std::thread (&LogicWork::deal_work, this,i);
	register_callback ();
	}
	catch (std::exception e) {
		Assert (e.what ());
	}
}

LogicWork::~LogicWork ()
{
	m_b_stop = true;
	m_cond.notify_all ();
	for (auto&& thr : m_work_thr)
		thr.join ();
}

void LogicWork::post_work(LogicNodePtr_t work_, std::size_t hash_code_)
{
	std::lock_guard l(m_mutex);
	DEBUGLOG("投递任务")
	m_work_map[hash_code_].push (std::move (work_));
	m_cond.notify_one();
}

void LogicWork::deal_work(std::size_t index_) {
	auto&& work_que = m_work_map[index_];
	LogicNodePtr_t node;
	while (true)
	{
		{
			std::unique_lock l (m_mutex);
			m_cond.wait (l, [this, &work_que] {
				return m_b_stop || !work_que.empty ();
				});
			if (m_b_stop && work_que.empty())
				return;
			if (!work_que.empty ()) { // 队列非空, 处理数据
				node = std::move (work_que.front ());
				work_que.pop ();
			}
		}
		auto msg_id = node->m_recv_node->get_msgID ();
		if (m_callback_handler.contains (msg_id))
			m_callback_handler[msg_id] (node->m_session, msg_id, node->m_recv_node->body);
		else INFOLOG (std::format ("not found callback, id:{}", enum_to_int (msg_id)));

	}

}
// 此为旧的逻辑线程功能, 这个操作现在转移到FileSystem的工作者线程中进行了
//void LogicWork::do_upload_file_req(CSessionPtr_t node, ReqId id, const std::string& data)
//{
//}
void LogicWork::test_send (CSessionPtr_t session, ReqId id, const std::string& data)
{
	spdlog::info (std::format ("test send msg:{}", data));
	(void) id;
	session->send (ReqId::E_ID_TEST_MSG_RSP, data);
}
void LogicWork::register_callback()
{
	m_callback_handler[ReqId::E_ID_TEST_MSG_REQ] = std::bind (&LogicWork::test_send, this, _1, _2, _3);
	m_callback_handler[ReqId::E_ID_UPLOAD_FILE_REQ] = [this](CSessionPtr_t session, ReqId id, const std::string& msg_data) {
		try{
			json::json root = json::json::parse(msg_data);
			json::json respond;
			Defer d([&session, &respond] {
				session->send(ReqId::E_ID_UPLOAD_FILE_RSP, respond.dump());
				});
			if (root.is_null() && !root.is_object())
			{
				respond["success"] = enum_to_int(ResStaus::E_INVALID_JSON);
				respond["message"] = "json is invalid!!!";
				return;
			}
			std::string md5 = root["md5"];
			std::string filename = root["name"]; // 文件名字
			std::size_t seq = root["seq"]; // 第几次发送
			std::size_t trans_size = root["trans_size"];
			std::size_t total_size = root["total_size"];
			bool last = root["last"]; // 是不是最后一个包
			std::size_t offset = root["offset"];
			std::string base64_buffer = root["data"];
			std::string data = base64_decode(base64_buffer);
			Assert(data != "", std::format("file data except, data len:{}", base64_buffer.size()));
			std::optional path_pre = cfg["Path"]["ResourceRoot"].value<std::string>();
			//Assert(path_pre.has_value(), "resource file path not found in config"); // 检查位置是否存在, 此操作在工作者线程中有, 暂时忽略

			fs::path resource_path (path_pre.value ());
			if (!fs::exists(resource_path)) {
				DEBUGLOG (std::format ("not found path:{}, creat path now.", resource_path.string ()));
				fs::create_directories (resource_path.parent_path ());
			}
			resource_path /= filename;
			//if (resource_path)
			// 通过文件名hash值与filesystem的工作者数量取余计算放入的位置
			std::size_t index = std::hash<std::string>()(filename) % FileSystem::s_logic_worker_count;
			// 向工作者投递任务
			FileSystem::get_instance().post_task_to_worker(std::make_unique<FileSystem::FileTask>(
				session, seq, total_size, trans_size, filename, resource_path, data,  offset, last
			), index);
		}
		catch (std::exception e) {
			Assert(e.what());
		}

		};
}



