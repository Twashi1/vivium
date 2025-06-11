#include "log.h"
#include "../system/os.h"

namespace Vivium {
	const char* getSeverityName(LogSeverity severity)
	{
		switch (severity) {
		case LogSeverity::DEBUG: return "DEBUG";
		case LogSeverity::WARN: return "WARN";
		case LogSeverity::ERROR: return "ERROR";
		case LogSeverity::FATAL: return "FATAL";
		default:
			return "INVALID";
		}
	}

#ifdef VIVIUM_PLATFORM_WINDOWS
	void _activateVirtualTerminal()
	{
		Windows::HANDLE handleOutput = Windows::GetStdHandle(Windows::_STD_OUTPUT_HANDLE);

		if (handleOutput == Windows::_INVALID_HANDLE_VALUE) return;

		Windows::DWORD originalOutputMode = 0;

		if (!Windows::GetConsoleMode(handleOutput, &originalOutputMode)) return;

		// TODO: macros should be specially defined
		Windows::DWORD requestedOutputModes = originalOutputMode
			| Windows::_ENABLE_VIRTUAL_TERMINAL_PROCESSING
			| Windows::_DISABLE_NEWLINE_AUTO_RETURN;

		if (!Windows::SetConsoleMode(handleOutput, requestedOutputModes)) return;

		_logState.isColorEnabled = true;
	}
#endif
		
	void _logInit()
	{
#ifdef VIVIUM_PLATFORM_WINDOWS
		_activateVirtualTerminal();
#endif
		_logState.logCallback = _defaultLogCallback;
	}
		
	std::string _setLogTextColor(std::string const& text, LogColor color)
	{
		return std::format("\033[{}m{}\033[{}m", (int)color + 30, text, (int)LogColor::NONE + 1);
	}
		
	void _defaultLogCallback(LogContext const& context)
	{
		std::cout << _defaultFormatLog(context);

		if (context.severity == LogSeverity::FATAL)
			std::terminate();
	}
		
	std::string _defaultFormatLog(LogContext const& context)
	{
		LogColor color = LogColor::NONE;

		switch (context.severity) {
		case LogSeverity::WARN: color = LogColor::YELLOW; break;
		case LogSeverity::FATAL: // Same as error
		case LogSeverity::ERROR: color = LogColor::RED; break;
		case LogSeverity::DEBUG: color = LogColor::GREEN; break;
		default: break;
		}

		return _setLogTextColor(std::format("[{}] ({}) {}: {}\n",
			Time::getTimestampString(context.timestamp),
			getSeverityName(context.severity),
			context.functionSignature,
			context.message
		), color);
	}

	void setLogCallback(LogCallback callback)
	{
		_logState.logCallback = callback;
	}
}