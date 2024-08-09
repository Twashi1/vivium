#include "state.h"

void ecs_test(void) {
	// Create million entities with int components
	Registry reg;

	std::vector<Entity> entities;

	for (int i = 0; i < 1'000; i++) {
		Entity e = reg.create();

		int copy = i;

		reg.addComponent<int>(e, std::move(copy));
		if (i % 4 == 1)
			reg.addComponent<std::string>(e, "Hello");
	}

	auto view = reg.createView<Owned<int>, Owned<std::string>>();

	for (auto& element : view) {
		VIVIUM_LOG(Log::DEBUG, "Val: {}, text: {}", element.get<int>(), element.get<std::string>());
	}
}

int main(void) {
	ecs_test();

	State state;

	initialise(state);
	gameloop(state);
	terminate(state);

	return NULL;
}