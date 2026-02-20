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

struct Point {
  F32x2 current;
  F32x2 previous;
  F32x2 acceleration;

  float radius;
  Color color;
};

Point createPoint(F32x2 pos, float radius, Color color);
void updatePoint(Point& point, float dt);

void init(State* state);
void run(State* state);
void drop(State* state);
}  // namespace Verlet
