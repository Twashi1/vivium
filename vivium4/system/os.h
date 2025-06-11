#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define VIVIUM_PLATFORM_WINDOWS
#elif defined(__linux__)
#define VIVIUM_PLATFORM_LINUX
#else
static_assert(false, "Invalid build platform");
#endif

namespace Vivium {
#ifdef VIVIUM_PLATFORM_WINDOWS
	namespace Windows {
		#undef _MSC_EXTENSIONS
		#define _WIN32_WINNT 0x0601
		#define NOMINMAX
		#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
		#include <synchapi.h>

		inline const DWORD _STD_OUTPUT_HANDLE = STD_OUTPUT_HANDLE;
		inline const DWORD _STATUS_WAIT_0 = STATUS_WAIT_0;
		inline const DWORD _WAIT_OBJECT_0 = WAIT_OBJECT_0;
		inline const HANDLE _INVALID_HANDLE_VALUE = INVALID_HANDLE_VALUE;

		inline const DWORD _ENABLE_VIRTUAL_TERMINAL_PROCESSING = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		inline const DWORD _DISABLE_NEWLINE_AUTO_RETURN = DISABLE_NEWLINE_AUTO_RETURN;

		#undef ERROR
		#undef MOD_SHIFT
		#undef MOD_ALT
		#undef MOD_CONTROL
		#undef RELATIVE

		#undef STD_OUTPUT_HANDLE
		#undef STATUS_WAIT_0
		#undef WAIT_OBJECT_0
		#undef INVALID_HANDLE_VALUE

		#undef ENABLE_VIRTUAL_TERMINAL_PROCESSING
		#undef DISABLE_NEWLINE_AUTO_RETURN
	}
// TODO: linux alternative (nanosleep and coloured terminal)
#endif
}
