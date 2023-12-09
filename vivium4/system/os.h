#pragma once

namespace Vivium {
	namespace Windows {
		#undef _MSC_EXTENSIONS
		#define _WIN32_WINNT 0x0601
		#define NOMINMAX
		#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
		#include <synchapi.h>

		inline constexpr DWORD _STD_OUTPUT_HANDLE		= STD_OUTPUT_HANDLE;
		inline constexpr DWORD _STATUS_WAIT_0			= STATUS_WAIT_0;
		inline constexpr DWORD _WAIT_OBJECT_0			= WAIT_OBJECT_0;
		inline constexpr HANDLE _INVALID_HANDLE_VALUE	= INVALID_HANDLE_VALUE;
	}
}

#undef ERROR
#undef MOD_SHIFT
#undef MOD_ALT
#undef MOD_CONTROL

#undef STD_OUTPUT_HANDLE
#undef STATUS_WAIT_0
#undef WAIT_OBJECT_0
#undef INVALID_HANDLE_VALUE