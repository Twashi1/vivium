#include "../vivium4/vivium4.h"
#include "state.h"

int main(int argc, char* argv[]) {
  Verlet::State* state = new Verlet::State();
  Verlet::init(*state);
  Verlet::run(*state);
  Verlet::drop(*state);

  delete state;
}
