#include "log.h"
#include "../system/os.h"

namespace Vivium {
	namespace Log {
		const char* getSeverityName(Severity severity)
		{
			switch (severity) {
			case DEBUG: return "DEBUG";
			case WARN: return "WARN";
			case ERROR: return "ERROR";
			case FATAL: return "FATAL";
			case INVALID:
			default:
				return "INVALID";
			}
		}

		Color getSeverityColor(Severity severity)
		{
			switch (severity) {
			case WARN: return Color::YELLOW;
			case FATAL:
			case ERROR: return Color::RED;
			case DEBUG: return Color::GREEN;
			case INVALID: 
			default: 
				return Color::NONE;
			}
		}

		void m_activeVirtualTerminal()
		{
			Windows::HANDLE handleOutput = Windows::GetStdHandle(Windows::_STD_OUTPUT_HANDLE);

			if (handleOutput == Windows::_INVALID_HANDLE_VALUE) return;

			Windows::DWORD originalOutputMode = 0;

			if (!Windows::GetConsoleMode(handleOutput, &originalOutputMode)) return;

			Windows::DWORD requestedOutputModes = originalOutputMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;

			if (!Windows::SetConsoleMode(handleOutput, requestedOutputModes)) return;

			m_state.isColorEnabled = true;
		}
		
		void m_init()
		{
			// TODO: if windows
			m_activeVirtualTerminal();
		}
		
		std::string m_setColor(const std::string& text, Color color)
		{
			return std::format("\033[{}m{}\033[{}m", (int)color + 30, text, (int)Color::NONE + 1);
		}
		
		void m_defaultLogCallback(const Context& context)
		{
			if (context.severity == Severity::DEBUG)
				std::cout << m_formatLog(context);

			if (context.severity >= Severity::WARN)
				std::cout << m_formatLog(context);

			if (context.severity == Severity::FATAL)
				std::terminate();
		}
		
		std::string m_formatLog(const Context& context)
		{
			return m_setColor(std::format("[{}] ({}) {}: {}\n",
				Time::getTimestampString(context.timestamp),
				getSeverityName(context.severity),
				context.functionSignature,
				context.message
			), getSeverityColor(context.severity));
		}
	}
}