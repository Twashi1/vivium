#pragma once

#include <format>
#include <iostream>
#include <string>

#include "../time/timer.h"

#if defined(__GNUC__) || defined(__GNUG__)
#define VIVIUM_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define VIVIUM_PRETTY_FUNCTION __FUNCSIG__
#else
static_assert("Failed to detect compiler")
#endif

namespace Vivium {
enum class LogSeverity { DEBUG, WARN, ERROR, FATAL };

/*! \brief Enum helper to get a string for the severity level. */
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

  LogContext(LogSeverity severity, std::string message, uint32_t line,
             char const* functionSignature, char const* filename,
             std::chrono::system_clock::time_point time);
};

typedef void (*LogCallback)(LogContext const&);
void _defaultLogCallback(LogContext const& context);
std::string _defaultFormatLog(LogContext const& context);
std::string getHexDump(void const* ptr, uint64_t n);

struct LogState {
  LogCallback logCallback;
  bool isColorEnabled;
};

/*! \brief Set a custom log callback for flexible logging.
 *
 * Allows redirection of logs to file, or custom printing and format.
 *
 * \param callback The function to call, expected to take a LogContext.
 */
void setLogCallback(LogCallback callback);

inline LogState _logState;
}  // namespace Vivium

#ifdef NDEBUG
#define VIVIUM_LOG(severity, message, ...) ((void)0)
#else
// Use VIVIUM_SOURCE_PATH_SIZE to advance the __FILE__ pointer to cut off the
// source path
#define VIVIUM_LOG(severity, message, ...)                                 \
  Vivium::_logState.logCallback(Vivium::LogContext(Vivium::LogContext(     \
      severity, std::format(message __VA_OPT__(, ) __VA_ARGS__), __LINE__, \
      VIVIUM_PRETTY_FUNCTION, __FILE__, std::chrono::system_clock::now())))
#endif
