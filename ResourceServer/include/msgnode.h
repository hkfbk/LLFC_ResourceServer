#pragma once
#include "global.hpp"
#include <boost/asio/detail/socket_ops.hpp>
namespace ops = boost::asio::detail::socket_ops;
struct MsgNodeBase
{
	friend class CSession;
	using msgLen_t = unsigned int;
	//using msgID_t = unsigned short;
	using msgID_t = ReqId;


	MsgNodeBase (char* msg_, msgLen_t len_);
	MsgNodeBase (msgLen_t len_);
	// 清空消息体，不删除
	void clear ();
	virtual ~MsgNodeBase ();
	// 消息体
	char* body;
	// 消息总长度
	msgLen_t total_len;
	// 消息当前长度
	msgLen_t current_len;
};
using MsgNode = MsgNodeBase;

struct RecvNode : public MsgNodeBase
{
	friend class CSession;
	using msgLen_t = unsigned int;
	RecvNode (const RecvNode&) = delete;
	RecvNode& operator=(const RecvNode&) = delete;
	RecvNode (msgLen_t len_, msgID_t id_);
	msgID_t get_msgID ()const;
	bool is_fulled () const noexcept;
private:
	msgID_t msg_id;
};


struct SendNode : public MsgNodeBase
{
	friend class CSession;
	using msgLen_t = unsigned int;
	SendNode (const SendNode&) = delete;
	SendNode& operator=(const SendNode&) = delete;
	SendNode (const char* msg_, msgLen_t len_, msgID_t id_);

private:
	msgID_t msg_id;
};