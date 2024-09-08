#include "state.h"

int main(void) {
	State state;

	initialise(state);
	gameloop(state);
	terminate(state);

	return NULL;
}