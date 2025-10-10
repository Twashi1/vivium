#pragma once

#include <chrono>
#include <format>
#include <string>

#include "../system/os.h"

namespace Vivium {
namespace Time {

/*! \brief Get the formatted HH:MM:SS of the timepoint passed.
 *
 * \param time The time to get the timestamp of.
 * \return The string of the timstamp.
 */
std::string getTimestampString(std::chrono::system_clock::time_point time);

#ifdef VIVIUM_PLATFORM_WINDOWS
struct SleepTimer {
  // Could be NULL on non-debug mode
  Windows::HANDLE waitableTimer;

  SleepTimer();
  ~SleepTimer();
};

// TODO: testing this creates the waitable timer and destroys it appropriately
inline thread_local SleepTimer m_sleepTimer;
#endif

// Returns if sleep went well or not, resolution of ~100ns
// https://stackoverflow.com/questions/13397571/precise-thread-sleep-needed-max-1ms-error

/*! \brief Short-duration sleep for frame-rate limiting.
 *
 * Implementation is per-platform due to requiring high resolution (~100ns).
 *
 * \param nanoseconds The number of nanoseconds to sleep for.
 * \return Returns if the sleep succeeded.
 */
bool nanosleep(long long nanoseconds);

struct Timer {
 private:
  using clock_type = std::chrono::steady_clock;

  clock_type::time_point m_time;
  // We use the sign bit to indicate if its just been resumed
  // Where = 0 means unpaused
  //		 < 0 means to be resumed
  //		 > 0 means currently paused
  float m_secondsElapsedDuringPause = 0.0f;

  static float m_getSecondsBetween(clock_type::time_point start,
                                   clock_type::time_point end);

  float m_getElapsedWithoutPause() const;

 public:
  Timer();

  /*! \brief Get time since last reset or initialisation.
   *
   * Does not factor in time spent in pause. Resolution is some number of
   * milliseconds.
   *
   * \return Number of seconds.
   */
  float getTime() const;
  /*! \brief Reset the timer to 0 and return time.
   *
   * \return Number of seconds since last reset or initialisation.
   */
  float reset();

  /*! \brief Pause the timer; time will not tick up. */
  void pause();
  /*! \brief Unpause the timer. */
  void resume();
  /*! \brief Return if the timer is currently paused. */
  bool isPaused() const;
};
}  // namespace Time
}  // namespace Vivium
