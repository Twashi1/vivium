#include "timer.h"

#include "../core.h"

namespace Vivium {
	namespace Time {
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
	}
}