#include "../vivium4/vivium4.h"

using namespace Vivium;

void groupTest() {
	_logInit();

	VIVIUM_LOG(LogSeverity::DEBUG, "Doing group test");

	Registry reg;

	reg.registerComponent<int>();
	reg.registerComponent<float>();

	// Create dummy entities
	constexpr uint64_t dummyCount = 1000;
	std::array<Entity, dummyCount> entities;

	for (uint64_t i = 0; i < dummyCount; i++) {
		entities[i] = reg.create();
		reg.addComponent<int>(entities[i], i);

		if (i % 2 == 0) {
			reg.addComponent<float>(entities[i], static_cast<float>(i));
		}
	}

	// Suboptimal usage, should do this before adding
	View<Owned<int>, Owned<float>> view = reg.createView<Owned<int>, Owned<float>>();

	// Ensure we have all even numbers, no duplicates, etc.
	std::array<bool, dummyCount> numbers;
	std::fill(numbers.begin(), numbers.end(), false);

	// Set some random odd numbers after view creation
	reg.addComponent<float>(entities[303], 303.0f);
	reg.addComponent<float>(entities[751], 751.0f);
	// 753 shouldn't show up
	reg.addComponent<float>(entities[753], 753.0f);
	reg.removeComponent<float>(entities[753]);

	for (ViewElement<Owned<int>, Owned<float>>& element : view) {
		int curr = element.get<int>();

		if (curr % 2 == 1) {
			VIVIUM_ASSERT(curr == 303 || curr == 751, "Invalid entity included in view: {}", curr);
		}

		VIVIUM_ASSERT(!numbers[curr], "Found duplicate element in view: {}", curr);
		
		numbers[curr] = true;

		VIVIUM_ASSERT(curr == static_cast<int>(element.get<float>()), "Float part not equal, didn't have float part?");
	}

	// Ensure we got all even numbers and 303, 751
	for (int i = 0; i < 1000; i++) {
		if (i == 303 || i == 751) {
			VIVIUM_ASSERT(numbers[i], "Didn't find special {}", i);
		}
		else {
			// Either its set and not odd, or its not set and is odd
			VIVIUM_ASSERT(numbers[i] + (i % 2 == 1) == 1, "Index {} was set invalid", i);
		}
	}

	VIVIUM_LOG(LogSeverity::DEBUG, "Group test successful");
}