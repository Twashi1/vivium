#pragma once

#include <iostream>
#include <format>
#include <string>

#include "../time/timer.h"

namespace Vivium {
	enum class LogSeverity {
		DEBUG,
		WARN,
		ERROR,
		FATAL
	};

	const char* getSeverityName(LogSeverity severity);

	enum class LogColor {
		NONE = -1,
		BLACK,
		RED,
		GREEN,
		YELLOW,
		BLUE,
		MAGENTA,
		CYAN,
		WHITE
	};

#ifdef VIVIUM_PLATFORM_WINDOWS
	// https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#EXAMPLE_OF_ENABLING_VIRTUAL_TERMINAL_PROCESSING
	void _activateVirtualTerminal();
#endif

	void _logInit();
	std::string _setLogTextColor(std::string const& text, LogColor color);

	struct LogContext {
		LogSeverity severity;
		std::string message;

		uint32_t line;
		const char* functionSignature;
		const char* filename;

		std::chrono::system_clock::time_point timestamp;
	};

	typedef void(*LogCallback)(LogContext const&);
	void _defaultLogCallback(LogContext const& context);
	std::string _defaultFormatLog(LogContext const& context);

	struct LogState {
		LogCallback logCallback;
		bool isColorEnabled;
	};

	void setLogCallback(LogCallback callback);

	inline LogState _logState;
}

#ifdef NDEBUG
#define VIVIUM_LOG(severity, message, ...) ((void)0)
#else
// Use VIVIUM_SOURCE_PATH_SIZE to advance the __FILE__ pointer to cut off the source path
#define VIVIUM_LOG(severity, message, ...) \
	Vivium::_logState.logCallback( \
		Vivium::LogContext{ \
			severity, std::format(message, __VA_ARGS__), __LINE__, __FUNCSIG__, __FILE__ + VIVIUM_SOURCE_PATH_SIZE, std::chrono::system_clock::now() \
		} \
	)
#endif