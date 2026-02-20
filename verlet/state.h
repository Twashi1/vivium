#pragma once

#include "../vivium4/vivium4.h"

using namespace Vivium;

namespace Verlet {
struct State {
  Engine engine;
  Window window;
  CommandContext context;
  ResourceManager manager;
};

void init(State* state);
void run(State* state);
void drop(State* state);
}  // namespace Verlet
