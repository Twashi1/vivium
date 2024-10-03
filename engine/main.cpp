#include "state.h"
#include "ecstest.h"

void game() {
	State state;

	initialise(state);
	gameloop(state);
	terminate(state);
}

void ecs() {
	groupTest();
}

int main(void) {
	ecs();

	return NULL;
}