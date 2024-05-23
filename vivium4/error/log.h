#pragma once

#include <iostream>
#include <format>
#include <string>

#include "../time/timer.h"

namespace Vivium {
	namespace Log {
		enum Severity {
			INVALID,
			DEBUG,
			WARN,
			ERROR,
			FATAL
		};

		const char* getSeverityName(Severity severity);

		enum Color {
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

		Color getSeverityColor(Severity severity);

		// Windows
		// https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#EXAMPLE_OF_ENABLING_VIRTUAL_TERMINAL_PROCESSING
		void m_activeVirtualTerminal();
		void m_init();

		// TODO: forgot where i yoinked this from
		std::string m_setColor(const std::string& text, Color color);

		struct Context {
			Severity severity;
			std::string message;

			uint32_t line;
			const char* functionSignature;
			const char* filename;

			std::chrono::system_clock::time_point timestamp;
		};

		typedef void(*LogCallback)(const Context&);
		void m_defaultLogCallback(const Context& context);
		std::string m_formatLog(const Context& context);

		struct LoggerState {
			LogCallback logCallback;
			bool isColorEnabled;

			LoggerState()
				: logCallback(m_defaultLogCallback), isColorEnabled(false)
			{}
		};

		inline LoggerState m_state;

		void setLogCallback(LogCallback callback);
	}
}

#define VIVIUM_CONTEXT(severity, message) \
	Vivium::Log::Context{severity, message, __LINE__, __FUNCSIG__, __FILE__, std::chrono::system_clock::now()}
#define VIVIUM_LOG(severity, message, ...) \
	Vivium::Log::m_state.logCallback(VIVIUM_CONTEXT(severity, std::format(message, __VA_ARGS__)))
#ifdef NDEBUG
#define VIVIUM_DEBUG_LOG(message, ...) ((void)0);
#else
#define VIVIUM_DEBUG_LOG(message, ...) VIVIUM_LOG(Log::DEBUG, message, __VA_ARGS__)
#endif