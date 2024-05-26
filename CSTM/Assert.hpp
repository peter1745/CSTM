#pragma once

#if defined(_WIN32)
	#define TheiaDebugBreak __debugbreak()
#elif __has_include(<csignal>)
	#include <csignal>
	#define TheiaDebugBreak raise(SIGTRAP)
#endif

#define TheiaAssert(expr) do {\
	if (!(expr))\
	{\
		/* TODO(Peter): Log / print error somewhere */ \
		TheiaDebugBreak;\
	}\
} while (false)

#define TheiaToDo() TheiaAssert(false)
