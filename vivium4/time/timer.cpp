#include "timer.h"

#include "../core.h"
#include "../error/log.h"

namespace Vivium {
	namespace Time {
#ifdef VIVIUM_PLATFORM_WINDOWS
		SleepTimer::SleepTimer() {
			waitableTimer = Windows::CreateWaitableTimerExW(
				NULL,
				NULL,
				CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
				TIMER_ALL_ACCESS
			);

			VIVIUM_ASSERT(waitableTimer != NULL, "Created NULL waitable timer");
		}
		
		SleepTimer::~SleepTimer() {
			if (waitableTimer != NULL) Windows::CloseHandle(waitableTimer);
		}
#endif
		
		std::string getTimestampString(std::chrono::system_clock::time_point time)
		{
			return std::format("{:%H:%M:%OS}", time);
		}

#if defined(VIVIUM_PLATFORM_WINDOWS)
		bool nanosleep(long long nanoseconds)
		{
			Windows::LARGE_INTEGER li;

			// Set timer properties
			li.QuadPart = -(nanoseconds / 100);

			if (!SetWaitableTimer(m_sleepTimer.waitableTimer, &li, 0, NULL, NULL, FALSE)) {
				VIVIUM_LOG(LogSeverity::FATAL, "Failed to set waitable timer");

				return FALSE;
			}

			// Start and wait for timer
			Windows::DWORD ret = Windows::WaitForSingleObject(m_sleepTimer.waitableTimer, INFINITE);

			VIVIUM_ASSERT(ret == Windows::_WAIT_OBJECT_0, "Failed to wait for timer");

			return TRUE;
		}
#elif defined(VIVIUM_PLATFORM_LINUX)
		// https://stackoverflow.com/questions/7684359/how-to-use-nanosleep-in-c-what-are-tim-tv-sec-and-tim-tv-nsec
		// NOTE: untested, don't have a linux machine, nor desire to build for linux just yet
		bool nanosleep(long long nanoseconds)
		{
			timespec time;
			time.tv_nsec = nanoseconds;

			if (::nanosleep(&time, nullptr) < 0) {
				// Sleep failed
				return false;
			}

			return true;
		}
#endif
		
		float Timer::m_getSecondsBetween(clock_type::time_point start, clock_type::time_point end)
		{
			return std::chrono::duration<float>(
				end - start
			).count();
		}
		
		float Timer::m_getElapsedWithoutPause() const
		{
			return m_getSecondsBetween(m_time, std::chrono::steady_clock::now());
		}
		
		Timer::Timer()
			: m_time(std::chrono::steady_clock::now())
		{}
		
		float Timer::getTime() const
		{
			return m_secondsElapsedDuringPause > 0.0f ?
				m_secondsElapsedDuringPause :
				m_getElapsedWithoutPause() - m_secondsElapsedDuringPause;
		}
		
		float Timer::reset()
		{
			float elapsed = getTime();

			m_time = std::chrono::steady_clock::now();

			if (m_secondsElapsedDuringPause < 0.0f) m_secondsElapsedDuringPause = 0.0f;

			return elapsed;
		}
		
		void Timer::pause()
		{
			m_secondsElapsedDuringPause = getTime();
		}
		
		void Timer::resume()
		{
			m_time = std::chrono::steady_clock::now();
			m_secondsElapsedDuringPause = -m_secondsElapsedDuringPause;
		}
		
		bool Timer::isPaused() const
		{
			return m_secondsElapsedDuringPause != 0.0f;
		}
	}
}