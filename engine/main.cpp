#include "ecstest.h"
#include "state.h"

void editor() {
  State* state = new State();

  initialise(*state);
  gameloop(*state);
  terminate(*state);

  delete state;
}

void ecs() { groupTest(); }

int main(void) {
  editor();

  return NULL;
}
