#include "../vivium4/vivium4.h"
#include "runtime.h"

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
		::State* state = new State();
		::initialise(*state);
		::gameloop(*state);
		::terminate(*state);

		delete state;
	}

	return NULL;
}