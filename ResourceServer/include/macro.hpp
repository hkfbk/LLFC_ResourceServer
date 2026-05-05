#pragma once

#ifndef _NOCOPY_
#define _NOCOPY_(T) T(const T&) = delete;\
T& operator=(const T&) = delete
#endif // _NOCOPY_

#ifndef __NOCOPY_MOVE__
#define __NOCOPY_MOVE__(T) _NOCOPY_(T);\
T(const T&&) = delete; \
T& operator=(const T&&) = delete
#endif // !__NOCOPY_MOVE__


#if defined(_MSC_VER)
#define ALWAYS_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#else
#define ALWAYS_INLINE inline
#endif