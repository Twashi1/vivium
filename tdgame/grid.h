#pragma once

#include "../vivium4/vivium4.h"

using namespace Vivium;

namespace TDGame {
enum GridType { SLOT, TEST, GRASS };
struct State;

struct _GridInstanceData {
  F32x2 position;
  F32x2 size;
  F32x2 texturePosition;
  F32x2 textureSize;
};

struct Level {
  std::vector<GridType> grid;

  Ref<Texture> texture;
  Ref<Buffer> gridStorageBuffer;
  Ref<Shader> gridVertexShader;
  Ref<Shader> gridFragmentShader;
  Ref<DescriptorSet> gridDescriptorSet;
  Ref<DescriptorLayout> gridDescriptorLayout;
  Ref<Pipeline> gridPipeline;
};

void submitLevel(Level& level, State& state);
void setupLevel(Level& level, State& state);
void prepareLevel(Level& level, State& state);
void renderLevel(Level& level, State& state);
void dropLevel(Level& level, State& state);
}  // namespace TDGame
