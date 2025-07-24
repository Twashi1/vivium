#include "state.h"
#include "ecstest.h"

void editor() {
	State* state = new State();

	initialise(*state);
	gameloop(*state);
	terminate(*state);

	delete state;
}

void ecs() {
	groupTest();
}

int main_old(void) {
	editor();

	return NULL;
}