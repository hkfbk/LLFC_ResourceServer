#pragma once
#include <functional>
#include <memory>
#include <thread>
#include <queue>
#include <condition_variable>
#include <map>
#include <unordered_map>
#include "global.hpp"
#include "msgnode.h"
#include "singleton.hpp"

class CSession;
struct LogicNode // 定义逻辑节点，以供逻辑系统使用
{
	std::shared_ptr<CSession> m_session;
	std::unique_ptr<RecvNode> m_recv_node;
	inline LogicNode (std::shared_ptr<CSession> session, std::unique_ptr<RecvNode>&& recv_node)
		:m_session (session), m_recv_node (std::move (recv_node))
	{ }
};

class LogicWork :public Singleton<LogicWork>
{
	using CallBack = std::function<void(std::shared_ptr<CSession>, ReqId, const std::string&)>;
	using LogicNodePtr_t = std::unique_ptr<LogicNode>;
	using CSessionPtr_t = std::shared_ptr<CSession>;
	DEFINE_SINGLETON (LogicWork)
public:
	LogicWork();
	~LogicWork ();

	void post_work(LogicNodePtr_t work_, std::size_t hash_code_);
	void deal_work(std::size_t index_);
	void test_send (CSessionPtr_t session, ReqId id, const std::string& data);
	// 注册回调
	void register_callback();
	//// 处理上传逻辑 // 此为旧的逻辑线程功能, 这个操作现在转移到FileSystem的工作者线程中进行了
	//void do_upload_file_req(CSessionPtr_t node, ReqId id, const std::string& data);

private:
	std::map<ReqId, CallBack> m_callback_handler;
	std::queue<LogicNodePtr_t> m_work_que;
	std::unordered_map<std::size_t, std::queue<LogicNodePtr_t>> m_work_map;
public:
	static constexpr std::size_t s_work_logic_count = 2;
private:
	std::mutex m_mutex;
	std::condition_variable m_cond;
	std::thread m_work_thr[s_work_logic_count];
	bool m_b_stop;
};
