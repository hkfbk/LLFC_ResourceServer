#pragma once
/**
* @file global.hpp
* @author jhx
* @brief 共公共数据定义，常量、枚举等
* @date 2025/12/04
* @version 0.0.1
*/
#include <cstdint>
#include <type_traits>
#include <format>
#include <source_location>
#include <chrono>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <toml++/toml.hpp>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include "macro.hpp"
// 定义数据单位
constexpr std::int16_t B = 8;
constexpr std::int32_t KB = 1024 * B;
// 头部总长度
constexpr std::int16_t HEAD_TOTAL_LEN = 6;
using MsgHeadLen_t = std::int16_t;
// 同步id长度
constexpr std::int16_t HEAD_ID_LEN = 2;
using MsgId_t = std::int16_t;
// 头部数据长度
constexpr std::int32_t HEAD_DATA_LEN = 4;
using HeadData_t = std::int32_t;
// 接受队列最大个数
constexpr std::size_t  MAX_RESVQUE = 20000000;
// 发送队列最大个数
constexpr std::size_t MAX_SENDQUE = 20000000;
// 缓冲区总长度
constexpr std::uint32_t BUFFER_TOTAL_LEN = 1024 * 2;
using MsgLen_t = std::uint32_t;
// 消息总长度
constexpr std::uint32_t MSG_TOTAL_LEN = BUFFER_TOTAL_LEN;
// 文件单次发送的最大长度
constexpr std::int16_t MAX_FILE_LEN = 2 * B;



// 接受的消息id
enum class ReqId :std::uint16_t
{
	E_ID_TEST_MSG_REQ = 1001,
	E_ID_TEST_MSG_RSP = 1002,
	E_ID_UPLOAD_FILE_REQ = 1003, // 上传文件请求
	E_ID_UPLOAD_FILE_RSP = 1004, // 上传文件回包

};

enum class ResStaus :std::uint16_t
{
	E_SUCESS = 0, // 请求成功
	E_INVALID_ID = 1, // 请求失败, 无效消息id
	E_INVALID_JSON = 3, // 请求失败， 无效json
};

template<typename E, std::size_t s>
concept EnumWithSize = std::is_enum_v<E> && sizeof (std::underlying_type_t<E>) == s;

template<typename E>
concept Enum16 = EnumWithSize<E, 2>;

template <Enum16 E>
constexpr ALWAYS_INLINE auto enum_to_int (E e) { return static_cast<std::underlying_type_t<E>> (e); }


template<typename F>
struct Defer
{
	Defer (F func)
		:m_func (func)
	{ }
	~Defer ()
	{
		m_func ();
	}
	F m_func;
};

ALWAYS_INLINE void Assert(std::string_view msg = "", std::source_location loca = std::source_location::current())
{
		spdlog::error(std::format("assert:{}, file:{},line:{}", msg, loca.file_name(), loca.line()));
#ifdef _DEBUG
		std::terminate();
#endif
}


ALWAYS_INLINE void Assert (bool e, std::string_view msg = "", std::source_location loca = std::source_location::current ())
{
	if (!e)
	{
		spdlog::error (std::format ("assert:{}, file:{},line:{}", msg, loca.file_name (), loca.line ()));
#ifdef _DEBUG
		std::terminate();
#endif
	}
}

inline static [[nodiscard]] ALWAYS_INLINE std::string base64_decode (const std::string& encoded_str)
{
	BIO* b64 = BIO_new (BIO_f_base64 ());
	BIO* bio = BIO_new_mem_buf (encoded_str.data (), -1);
	bio = BIO_push (b64, bio);
	// 不换行
	BIO_set_flags (bio, BIO_FLAGS_BASE64_NO_NL);
	// 解码
	std::string decoded;
	decoded.resize (encoded_str.size ());
	std::size_t bio_error_code = 0;
	std::size_t decoded_length = BIO_read_ex(bio, decoded.data(), decoded.size(), &bio_error_code);
	// 清理
	BIO_free_all (bio);
	if (decoded_length == 0)
		return { };
	return { decoded.data () , decoded_length };
}
namespace fs = std::filesystem;
inline static const auto&& cfg = toml::parse_file (fs::path ("config/config.toml").c_str ());