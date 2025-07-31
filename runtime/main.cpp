#include "../vivium4/vivium4.h"
#include "runtime.h"

void testing_ecs() {
	Registry* registry = new Registry;

	std::vector<Entity> entities;

	for (uint64_t i = 0; i < 1000; i++) {
		Entity e = registry->create();

		if (i % 2 == 0) {
			registry->addComponent<int>(e, i);
		}
		else {
			registry->addComponent<std::string>(e, std::format("hello {}", i));
		}

		entities.push_back(e);
	}

	SerialiserFileInterface myFile;
	myFile.begin("new.dat", false);

	serialiseWrite(*registry, myFile);

	myFile.end();

	// delete registry;

	Registry* newRegistry = new Registry;

	SerialiserFileInterface newFile;
	newFile.begin("new.dat", true);
	
	serialiseRead(newRegistry, newFile);

	// TODO: we lost all entity references once we deserialised the ECS...?
	//	we assume the user serialised the entity references in a meaningful way

	for (uint64_t i = 0; i < 1000; i++) {
		Entity e = entities[i];

		if (i % 2 == 0) {
			VIVIUM_ASSERT(newRegistry->hasComponent<int>(e), "Entity {} didn't have int component", (uint32_t)e);
			int value = newRegistry->getComponent<int>(e);

			VIVIUM_ASSERT(value == i, "Entity {} didn't match value to index {} = {}", (uint32_t)e, value, i);
		}
		else {
			VIVIUM_ASSERT(newRegistry->hasComponent<std::string>(e), "Entity {} didn't have string component", (uint32_t)e);
			std::string value = newRegistry->getComponent<std::string>(e);

			VIVIUM_ASSERT(value == std::format("hello {}", i), "Entity {} didn't match value to index {} = {}", (uint32_t)e, value, std::format("hello {}", i));
		}
	}

	newFile.end();

	delete newRegistry;
	delete registry;
}

#define TMP_EDITOR 0

int main(int argc, char* argv[]) {
	if (argc > 1) {
		// Attempt to run the inputted program
		char const* bytecodeFile = argv[2];

		Runtime::State* state = new Runtime::State();
		Runtime::init(*state, bytecodeFile);
		Runtime::run(*state);
		Runtime::drop(*state);

		delete state;
	}
	// Default to running editor
	else {
#if TMP_EDITOR
		::State* state = new State();
		::initialise(*state);
		::gameloop(*state);
		::terminate(*state);

		delete state;
#else
		Runtime::State* state = new Runtime::State();
		Runtime::init(*state, "gen.dat");
		Runtime::run(*state);
		Runtime::drop(*state);

		delete state;
#endif
	}

	return NULL;
}