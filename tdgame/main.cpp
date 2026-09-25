#include "state.h"

using namespace TDGame;

int main(void) {
  State* state = new State();

  initialise(*state);
  gameloop(*state);
  terminate(*state);

  delete state;

  return 0;
}
