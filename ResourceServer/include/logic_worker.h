#include <functional>
#include <memory>
#include <thread>
#include <queue>
#include <condition_variable>
#include <map>
#include "global.hpp"
#include "msgnode.h"


class CSession;
struct LogicNode // 定义逻辑节点，以供逻辑系统使用
{
	std::shared_ptr<CSession> m_session;
	std::shared_ptr<RecvNode> m_recv_node;
};

class LogicWork
{
	using CallBack = std::function<void(std::shared_ptr<CSession>, ReqId, const std::string&)>;
	//using LogicNodePtr_t = std::shared_ptr<LogicNode>;
	using CSessionPtr_t = std::shared_ptr<CSession>;
public:
	LogicWork();

	void post_work(CallBack work_);
	void deal_work();
	// 注册回调
	void register_callback();
	//// 处理上传逻辑 // 此为旧的逻辑线程功能, 这个操作现在转移到FileSystem的工作者线程中进行了
	//void do_upload_file_req(CSessionPtr_t node, ReqId id, const std::string& data);

private:
	std::map<ReqId, CallBack> m_callback_handler;
	std::queue<CallBack> m_work_que;
	std::mutex m_mutex;
	std::condition_variable m_cond;
	std::thread m_work_thr;
	bool m_b_stop;
};
