#pragma once
#include <string_view>
#ifndef _NOCOPY_
#define _NOCOPY_(T) T(const T&) = delete;\
T& operator=(const T&) = delete
#endif // _NOCOPY_

#ifndef __NOCOPY_MOVE__
#define __NOCOPY_MOVE__(T) _NOCOPY_(T);\
T(const T&&) = delete; \
T& operator=(const T&&) = delete
#endif // !__NOCOPY_MOVE__


#if defined(_MSC_VER) && !defined(__clang__) 
#define ALWAYS_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#else
#define ALWAYS_INLINE inline
#endif

ALWAYS_INLINE constexpr std::string_view ___debug_file_name(std::string_view str)
{
	auto index = str.find_last_of('\\');
	if (index != std::string_view::npos)
		return str.substr(index + 1);
	return str;
}

#ifdef _DEBUG
#ifndef DEBUGLOG
#define DEBUGLOG_ 1
#define DEBUGLOG(log) spdlog::debug("[{} {} {}] | {}", ___debug_file_name(__FILE__), __FUNCTION__, __LINE__, (log));
#endif // !DEBUGLOG
#ifndef LOG
#define LOG spdlog::info("[{} {} {}]", ___debug_file_name(__FILE__), __FUNCTION__, __LINE__);
#endif // !LOG
#else
#ifndef DEBUGLOG
#define DEBUGLOG(log);
#endif // !DEBUGLOG
#ifndef LOG
#define LOG ;
#endif // !LOG
#endif // !#if defined(_DEBUG)

#ifndef INFOLOG
#define INFOLOG(log) spdlog::info("[{} {} {}] | {}", ___debug_file_name(__FILE__), __FUNCTION__, __LINE__, (log));
#endif // !INFOLOG

#ifndef ERRORLOG
#define ERRORLOG(log) spdlog::error("[{} {} {}] | {}", ___debug_file_name(__FILE__), __FUNCTION__, __LINE__, (log));
#endif // !ERRORLOG