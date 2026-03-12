#include "msgnode.h"
#include "global.hpp"


MsgNodeBase::MsgNodeBase (char* msg_, msgLen_t len_)
	: total_len (len_ + HEAD_TOTAL_LEN)
	, current_len (0)
{
	body = new char[total_len + 1]; // 多申请一个字节用于存储/0
	auto net_lenght = ops::network_to_host_short (len_);
	std::memcpy (body, &net_lenght, HEAD_TOTAL_LEN); // 将头部消息拷贝到消息体中
	std::memcpy (body + HEAD_TOTAL_LEN, msg_, len_); // 将消息拷贝到消息体中
	body[total_len] = '\0';
}

MsgNodeBase::MsgNodeBase (msgLen_t len_)
	: total_len (len_)
	, current_len (0)
{
	body = new char[total_len + 1]; // 多申请一个字节用于存储/0
	body[total_len] = '\0';
}

void MsgNodeBase::clear ()
{
	std::memset (body, 0, total_len);
	current_len = 0;
}

MsgNodeBase::~MsgNodeBase ()
{
	delete[] body;
}


RecvNode::RecvNode (msgLen_t len_, msgID_t id_)
	: MsgNodeBase (len_)
	, msg_id (id_)
{ }

auto RecvNode::get_msgID () const ->msgID_t
{
	return msg_id;
}
bool RecvNode::is_fulled () const noexcept
{
	return total_len == current_len;
}

SendNode::SendNode (const char* msg_, msgLen_t len_, msgID_t id_)
	: MsgNodeBase (len_ + HEAD_TOTAL_LEN)
	, msg_id (id_)
{
	// 将消息逐步拷贝到消息体中
	auto host_id = ops::host_to_network_short (enum_to_int (id_)); // 将消息id转为网络字节序
	std::memcpy (body, &host_id, HEAD_ID_LEN);
	auto host_len = ops::host_to_network_long (len_); // 将消息体长度转为网络字节序
	std::memcpy (body + HEAD_ID_LEN, &host_len, HEAD_DATA_LEN);
	std::memcpy (body + HEAD_TOTAL_LEN, msg_, len_);
	current_len = len_ + HEAD_TOTAL_LEN;
}