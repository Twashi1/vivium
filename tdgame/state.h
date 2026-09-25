#pragma once

#include "../vivium4/vivium4.h"
#include "grid.h"

using namespace Vivium;

namespace TDGame {
struct State {
  Engine engine;
  Window window;
  CommandContext context;
  GUIContext guiContext;
  ResourceManager manager;

  StitchedAtlas guiAtlas;
  Level level;

  Ref<Buffer> rectVertexBuffer;
  Ref<Buffer> rectIndexBuffer;

  Perspective perspective;
};

void initialise(State& state);
void gameloop(State& state);
void terminate(State& state);

void submitState(State& state);
void setupState(State& state);
void dropState(State& state);
};  // namespace TDGame
