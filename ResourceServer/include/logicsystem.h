#pragma once
#include "global.hpp"
#include "singleton.hpp"
#include "csession.h"


#include <functional>
#include <queue>
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>

class LogicSystem : public Singleton<LogicSystem>
{
	friend Singleton<LogicSystem>;
	using CallBack = std::function<void (std::shared_ptr<CSession>, ReqId, const std::string&)>;
	using LogicNodePtr_t = std::shared_ptr<LogicNode>;
	using SessionPtr_t = std::shared_ptr<CSession>;
	using enum ReqId;
	_SLNGLETON_DELETE_COPY_MOVE_ (LogicSystem);
public:
	LogicSystem ();
	~LogicSystem ();
	void post_message_to_logic_system (LogicNodePtr_t node_);
private:
	// 处理消息
	void deal_message ();
	// 注册回调
	void register_callback ();
	// 测试逻辑
	void test_send (SessionPtr_t session, ReqId id, const std::string& data);
	// 处理长传逻辑
	void do_upload_file_req (SessionPtr_t session, ReqId id, const std::string& data);

private:
	// 定义节点和节点队列
	LogicNodePtr_t m_node;
	std::queue<LogicNodePtr_t> m_logic_que;

	// 定义回调
	std::map<ReqId, CallBack> m_callback_handler;
	// 定义锁和条件变量
	std::mutex m_mutex;
	std::condition_variable m_cond;

	// 定义工作线程
	std::thread m_word_thr;

	// 定义服务终止变量
	bool m_is_stop;
};