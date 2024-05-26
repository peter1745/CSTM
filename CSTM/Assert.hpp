#pragma once

#if defined(_WIN32)
	#define CSTM_DebugBreak __debugbreak()
#elif __has_include(<csignal>)
	#include <csignal>
	#define CSTM_DebugBreak raise(SIGTRAP)
#endif

#define CSTM_Assert(expr) do {\
	if (!(expr))\
	{\
		/* TODO(Peter): Log / print error somewhere */ \
		CSTM_DebugBreak;\
	}\
} while (false)

#define CSTM_ToDo() CSTM_Assert(false)
