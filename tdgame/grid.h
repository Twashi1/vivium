#pragma once

#include "../vivium4/vivium4.h"

using namespace Vivium;

namespace TDGame {
constexpr uint32_t MAX_GRID_SIZE = 32;

enum class RenderGridType : uint32_t {
  AIR,
  BALL,
  GRASS,
  PATH_HORIZONTAL,
  PATH_CORNER,
  PATH_TEE,
  PATH_INTERSECTION,
  START_SQUARE,
  END_SQUARE,
  TMP,
  TURRET,
  RED_ARROW,
  SLOT
};

struct SpriteAtlasRenderables {
  RenderGridType type;
  F32x2 position;
  float rotation;
};

enum class GridType : uint32_t {
  AIR,
  GRASS,
  DIRT_PATH,
  TURRET,
  START,
  END,
  __LAST
};

enum class PathDirection : uint32_t {
  NONE = 0x0,
  RIGHT = 0x1,
  LEFT = 0x2,
  UP = 0x4,
  DOWN = 0x8
};
struct State;

struct Stage {
  std::vector<GridType> grid;
  std::vector<PathDirection> exitDirections;
  std::vector<PathDirection> entryDirections;
  I32x2 dim;
  I32x2 start;
  I32x2 end;
};

struct _GridInstanceData {
  F32x2 position;
  F32x2 size;
  F32x2 texturePosition;
  F32x2 textureSize;
  float rotation;
  float _pad0;
  float _pad1;
  float _pad2;
};

struct Level {
  Registry registry;
  std::vector<Entity> gridEntities;

  Stage stage;

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
