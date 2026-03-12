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
