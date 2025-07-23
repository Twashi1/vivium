#include "../vivium4/vivium4.h"
#include "runtime.h"

int main_baaaad(int argc, char* argv[]) {
	if (argc > 1) {
		// Attempt to run the inputted program
		char const* bytecodeFile = argv[2];

		Runtime::State state;
		Runtime::init(state, bytecodeFile);
		Runtime::run(state);
		Runtime::drop(state);
	}
	// Default to running editor
	else {
		::State state;
		::initialise(state);
		::gameloop(state);
		::terminate(state);
	}

	return NULL;
}