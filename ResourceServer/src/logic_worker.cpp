#include "logic_worker.h"
#include <nlohmann/json.hpp>
#include "csession.h"
#include "file_system.h"
#include "global.hpp"
using namespace std::placeholders;
namespace json = nlohmann;

LogicWork::LogicWork()
	: m_work_que()
	, m_cond()
	, m_mutex()
{
	m_work_thr = std::thread(&LogicWork::deal_work, this);
}

void LogicWork::post_work(CallBack work_)
{
	std::lock_guard l(m_mutex);
	m_work_que.push(work_);
	m_cond.notify_one();
}

void LogicWork::deal_work() {

}
// 此为旧的逻辑线程功能, 这个操作现在转移到FileSystem的工作者线程中进行了
//void LogicWork::do_upload_file_req(CSessionPtr_t node, ReqId id, const std::string& data)
//{
//}

void LogicWork::register_callback()
{
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
			Assert(data == "", std::format("file data except"));
			//std::optional path_pre = cfg["Path"]["ResourceRoot"].value<std::string>();
			//Assert(path_pre.has_value(), "resource root is null"); // 检查位置是否存在, 此操作在工作者线程中有, 暂时忽略
			
			// 通过文件名hash值与filesystem的工作者数量取余计算放入的位置
			std::size_t index = std::hash<std::string>()(filename) % FileSystem::s_logic_worker_count; 
			// 向工作者投递任务
			FileSystem::get_instance().post_task_to_worker(std::make_shared<FileSystem::FileTask>(
				session, seq, total_size, trans_size, filename, data, offset, last
			), index);
		}
		catch (std::exception e) {
			Assert(e.what());
		}

		};
}



