#pragma once

#include <chrono>
#include <format>
#include <string>

#include "../system/os.h"

namespace Vivium {
	namespace Time {
		std::string getTimestampString(std::chrono::system_clock::time_point time);

		struct SleepTimer {
			// Could be NULL on non-debug mode
			Windows::HANDLE waitableTimer;

			SleepTimer();
			~SleepTimer();
		};

		// TODO: testing this creates the waitable timer and destroys it appropriately
		thread_local SleepTimer m_sleepTimer;

		// Returns if sleep went well or not, resolution of ~100ns
		// https://stackoverflow.com/questions/13397571/precise-thread-sleep-needed-max-1ms-error
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

			static float m_getSecondsBetween(clock_type::time_point start, clock_type::time_point end);

			float m_getElapsedWithoutPause() const;

		public:
			Timer();

			float getTime() const;
			// Returns elapsed time, and resets stopwatch
			float reset();

			void pause();
			void resume();
			bool isPaused() const;
		};
	}
}