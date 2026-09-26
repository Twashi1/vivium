#include "grid.h"

#include <sys/select.h>

#include <queue>

#include "state.h"

using namespace Vivium;

namespace TDGame {
static PathDirection flipDirection(PathDirection direction) {
  switch (direction) {
    case PathDirection::UP:
      return PathDirection::DOWN;
    case PathDirection::DOWN:
      return PathDirection::UP;
    case PathDirection::LEFT:
      return PathDirection::RIGHT;
    case PathDirection::RIGHT:
      return PathDirection::LEFT;
    default:
      return PathDirection::NONE;
  }
}

static void selectCorrectPathDirection(SpriteAtlasRenderables& renderable,
                                       PathDirection direction) {
  // Left 0001
  // Right 0010
  // Up 0100
  // Down 1000
  // Corner default is up-right
  // Tee default is left, up, down
  std::vector<RenderGridType> types = {
      RenderGridType::AIR,                // 0000
      RenderGridType::AIR,                // 0001 left
      RenderGridType::AIR,                // 0010 right
      RenderGridType::PATH_HORIZONTAL,    // 0011 left-right
      RenderGridType::AIR,                // 0100 up
      RenderGridType::PATH_CORNER,        // 0101 up-left
      RenderGridType::PATH_CORNER,        // 0110 up-right
      RenderGridType::PATH_TEE,           // 0111 up-left-right
      RenderGridType::AIR,                // 1000 down
      RenderGridType::PATH_CORNER,        // 1001 down-left
      RenderGridType::PATH_CORNER,        // 1010 down-right
      RenderGridType::PATH_TEE,           // 1011 down-left-right
      RenderGridType::PATH_HORIZONTAL,    // 1100 down-up
      RenderGridType::PATH_TEE,           // 1101 down-up-left
      RenderGridType::PATH_TEE,           // 1110 down-up-right
      RenderGridType::PATH_INTERSECTION,  // 1111 all
  };

  // left-down and up-right bad?

  std::vector<float> rotations = {
      0.0f,          // 0000
      0.0f,          // 0001 left
      0.0f,          // 0010 right
      0.0f,          // 0011 left-right
      0.0f,          // 0100 up
      0.5f * M_PI,   // 0101 up-left
      1.0f * M_PI,   // 0110 up-right
      -0.5f * M_PI,  // 0111 up-left-right
      0.0f,          // 1000 down
      0.0f * M_PI,   // 1001 down-left
      -0.5f * M_PI,  // 1010 down-right
      0.5f * M_PI,   // 1011 down-left-right
      0.5f * M_PI,   // 1100 down-up
      1.0f * M_PI,   // 1101 down-up-left
      0.0f,          // 1110 down-up-right
      0.0f           // 1111 all
  };

  uint32_t index = static_cast<uint32_t>(direction);
  renderable.type = types[index];
  renderable.rotation = rotations[index];

  VIVIUM_LOG(LogSeverity::DEBUG, "Selected index {}", index);
}

static PathDirection mergeDirections(PathDirection a, PathDirection b) {
  return static_cast<PathDirection>(static_cast<uint32_t>(a) |
                                    static_cast<uint32_t>(b));
}

static std::vector<SpriteAtlasRenderables> loadRenderablesGridOfStage(
    Stage& stage) {
  std::vector<SpriteAtlasRenderables> renderables;
  renderables.resize(MAX_GRID_SIZE * MAX_GRID_SIZE);

  for (uint32_t i = 0; i < MAX_GRID_SIZE * MAX_GRID_SIZE; i++) {
    SpriteAtlasRenderables renderable;
    renderable.type = RenderGridType::AIR;
    renderable.position = F32x2(0.0f);
    renderable.rotation = 0.0f;

    switch (stage.grid[i]) {
      case GridType::GRASS:
        renderable.type = RenderGridType::GRASS;
        break;
      case GridType::DIRT_PATH: {
        selectCorrectPathDirection(
            renderable,
            mergeDirections(stage.entryDirections[i], stage.exitDirections[i]));
        break;
      }
      case GridType::TURRET:
        renderable.type = RenderGridType::TURRET;
        break;
      case GridType::START:
        renderable.type = RenderGridType::START_SQUARE;
        break;
      case GridType::END:
        renderable.type = RenderGridType::END_SQUARE;
        break;
      default:
        break;
    }

    renderables[i] = std::move(renderable);
  }

  return renderables;
}

static void loadStage(Stage& stage, const std::string& path) {
  std::ifstream file(path);
  std::string line;

  uint32_t y = MAX_GRID_SIZE - 1;
  uint32_t x = 0;

  stage.grid.resize(MAX_GRID_SIZE * MAX_GRID_SIZE);
  std::fill(stage.grid.begin(), stage.grid.end(), GridType::AIR);

  stage.entryDirections.resize(MAX_GRID_SIZE * MAX_GRID_SIZE);
  std::fill(stage.entryDirections.begin(), stage.entryDirections.end(),
            PathDirection::NONE);
  stage.exitDirections.resize(MAX_GRID_SIZE * MAX_GRID_SIZE);
  std::fill(stage.exitDirections.begin(), stage.exitDirections.end(),
            PathDirection::NONE);

  std::vector<I32x2> starts;
  std::vector<I32x2> ends;

  while (std::getline(file, line)) {
    if (line.size() == 0) {
      x = 0;
      --y;
      continue;
    }

    for (char c : line) {
      switch (c) {
        case 's':
          stage.grid[x + y * MAX_GRID_SIZE] = GridType::START;
          starts.push_back(I32x2(x, y));
          break;
        case 'e':
          stage.grid[x + y * MAX_GRID_SIZE] = GridType::END;
          ends.push_back(I32x2(x, y));
          break;
        case 't':
          stage.grid[x + y * MAX_GRID_SIZE] = GridType::TURRET;
          break;
        case '#':
          stage.grid[x + y * MAX_GRID_SIZE] = GridType::GRASS;
          break;
        case '.':
          stage.grid[x + y * MAX_GRID_SIZE] = GridType::DIRT_PATH;
          break;
        default:
          stage.grid[x + y * MAX_GRID_SIZE] = GridType::AIR;
          break;
      }

      ++x;
    }

    x = 0;
    --y;
  }

  // Write directions in (BFS)
  std::vector<uint32_t> distances(MAX_GRID_SIZE * MAX_GRID_SIZE, UINT32_MAX);
  std::vector<I32x2> neighbourOffsets(4);
  neighbourOffsets[0] = I32x2(-1, 0);
  neighbourOffsets[1] = I32x2(1, 0);
  neighbourOffsets[2] = I32x2(0, -1);
  neighbourOffsets[3] = I32x2(0, 1);
  // We expect these to be in the same order as the offsets
  std::vector<PathDirection> neighbourDirections(4);
  neighbourDirections[0] = PathDirection::LEFT;
  neighbourDirections[1] = PathDirection::RIGHT;
  neighbourDirections[2] = PathDirection::UP;
  neighbourDirections[3] = PathDirection::DOWN;

  std::queue<uint32_t> queue;
  for (I32x2 end : ends) {
    int index = end.x + end.y * MAX_GRID_SIZE;
    queue.push(index);
    distances[index] = 0;
  }

  while (!queue.empty()) {
    uint32_t index = queue.front();
    queue.pop();

    int x = index % MAX_GRID_SIZE;
    int y = index / MAX_GRID_SIZE;

    for (uint32_t i = 0; i < neighbourOffsets.size(); i++) {
      I32x2 offset = neighbourOffsets[i];
      // Direction we're moving to get to this neighbour
      PathDirection direction = neighbourDirections[i];

      int nx = static_cast<int>(x) + offset.x;
      int ny = static_cast<int>(y) + offset.y;

      if (nx < 0 || nx >= MAX_GRID_SIZE || ny < 0 || ny >= MAX_GRID_SIZE) {
        continue;
      }

      int neighbourIndex = nx + ny * MAX_GRID_SIZE;

      if (stage.grid[neighbourIndex] != GridType::DIRT_PATH &&
          stage.grid[neighbourIndex] != GridType::END &&
          stage.grid[neighbourIndex] != GridType::START) {
        continue;
      }

      if (distances[neighbourIndex] == UINT32_MAX) {
        distances[neighbourIndex] = distances[index] + 1;
        queue.push(neighbourIndex);
      }
    }
  }

  for (uint32_t y = 0; y < MAX_GRID_SIZE; ++y) {
    for (uint32_t x = 0; x < MAX_GRID_SIZE; ++x) {
      uint32_t index = x + y * MAX_GRID_SIZE;

      if (distances[index] == UINT32_MAX) {
        continue;
      }

      for (uint32_t i = 0; i < neighbourOffsets.size(); ++i) {
        I32x2 offset = neighbourOffsets[i];
        PathDirection direction = neighbourDirections[i];

        int nx = static_cast<int>(x) + offset.x;
        int ny = static_cast<int>(y) + offset.y;

        if (nx < 0 || nx >= MAX_GRID_SIZE || ny < 0 || ny >= MAX_GRID_SIZE) {
          continue;
        }

        uint32_t neighbourIndex = nx + ny * MAX_GRID_SIZE;

        if (distances[neighbourIndex] == UINT32_MAX) {
          continue;
        }

        // Implies neighbour is at lower distance from the end
        // Hence the offset from current -> neighbour is moving towards the end
        if (distances[neighbourIndex] < distances[index]) {
          stage.exitDirections[index] =
              mergeDirections(direction, stage.exitDirections[index]);

          stage.entryDirections[neighbourIndex] = mergeDirections(
              flipDirection(direction), stage.entryDirections[neighbourIndex]);
        }
      }
    }
  }

  for (uint32_t y = 0; y < MAX_GRID_SIZE; ++y) {
    for (uint32_t x = 0; x < MAX_GRID_SIZE; ++x) {
      uint32_t index = x + y * MAX_GRID_SIZE;

      if (stage.grid[index] != GridType::DIRT_PATH) {
        continue;
      }

      PathDirection entry = stage.entryDirections[index];
      PathDirection exit = stage.exitDirections[index];
      PathDirection direction = mergeDirections(stage.entryDirections[index],
                                                stage.exitDirections[index]);

      VIVIUM_LOG(LogSeverity::DEBUG,
                 "Path at {} {} had direction index {}, entry {}, exit {}", x,
                 y, static_cast<uint32_t>(direction),
                 static_cast<uint32_t>(entry), static_cast<uint32_t>(exit));
    }
  }
}

void submitLevel(Level& level, State& state) {
  level.registry = Registry();
  level.registry.registerComponent<SpriteAtlasRenderables>();
  loadStage(level.stage, "tdgame/res/levels/tmp.txt");

  for (uint32_t i = 0; i < MAX_GRID_SIZE * MAX_GRID_SIZE; i++) {
    Entity entity = level.registry.create();
    level.gridEntities.push_back(entity);

    SpriteAtlasRenderables renderables;
    renderables.type = RenderGridType::AIR;
    renderables.position = F32x2(0.0f);
    renderables.rotation = 0.0f;

    level.registry.addComponent<SpriteAtlasRenderables>(entity,
                                                        std::move(renderables));
  }

  std::vector<SpriteAtlasRenderables> renderables =
      loadRenderablesGridOfStage(level.stage);
  for (uint32_t i = 0; i < MAX_GRID_SIZE * MAX_GRID_SIZE; i++) {
    level.registry.updateComponent<SpriteAtlasRenderables>(
        level.gridEntities[i], std::move(renderables[i]));
  }

  submitResource(
      state.manager, &level.texture.reference,
      std::vector<TextureSpecification>({TextureSpecification::fromImageFile(
          "tdgame/res/images/atlas.png", TextureFormat::RGBA,
          TextureFilter::NEAREST)}));
  submitResource(state.manager, &level.gridStorageBuffer.reference,
                 MemoryType::UNIFORM,
                 std::vector<BufferSpecification>({BufferSpecification(
                     MAX_GRID_SIZE * MAX_GRID_SIZE * sizeof(_GridInstanceData),
                     BufferUsage::STORAGE)}));

  submitResource(state.manager, &level.gridVertexShader.reference,
                 std::vector<ShaderSpecification>({compileShader(
                     ShaderStage::VERTEX, "tdgame/res/shaders/grid.vert",
                     "tdgame/res/shaders/grid_vert.spv")}));
  submitResource(state.manager, &level.gridFragmentShader.reference,
                 std::vector<ShaderSpecification>({compileShader(
                     ShaderStage::FRAGMENT, "tdgame/res/shaders/grid.frag",
                     "tdgame/res/shaders/grid_frag.spv")}));
  submitResource(state.manager, &level.gridDescriptorLayout.reference,
                 std::vector<DescriptorLayoutSpecification>(
                     {DescriptorLayoutSpecification(std::vector<UniformBinding>(
                         {UniformBinding(ShaderStage::VERTEX, 0,
                                         UniformType::STORAGE_BUFFER),
                          UniformBinding(ShaderStage::FRAGMENT, 1,
                                         UniformType::TEXTURE)}))}));
  submitResource(
      state.manager, &level.gridDescriptorSet.reference,
      std::vector<DescriptorSetSpecification>({DescriptorSetSpecification(
          level.gridDescriptorLayout.reference,
          std::vector<UniformData>(
              {UniformData::fromBuffer(
                   level.gridStorageBuffer.reference,
                   MAX_GRID_SIZE * MAX_GRID_SIZE * sizeof(_GridInstanceData),
                   0),
               UniformData::fromTexture(level.texture.reference)}))}));
  submitResource(
      state.manager, &level.gridPipeline.reference,
      std::vector<PipelineSpecification>({PipelineSpecification::fromWindow(
          std::vector<ShaderReference>({level.gridFragmentShader.reference,
                                        level.gridVertexShader.reference}),
          BufferLayout::fromTypes(std::vector<ShaderDataType>(
              {ShaderDataType::VEC2, ShaderDataType::VEC2})),
          std::vector<DescriptorLayoutReference>(
              {level.gridDescriptorLayout.reference}),
          std::vector<PushConstant>(
              {PushConstant(ShaderStage::VERTEX, 0, sizeof(Perspective))}),
          state.window)}));
}

void setupLevel(Level& level, State& state) {
  convertResourceReference(state.manager, level.gridStorageBuffer);
  convertResourceReference(state.manager, level.gridDescriptorLayout);
  convertResourceReference(state.manager, level.gridPipeline);
  convertResourceReference(state.manager, level.gridDescriptorSet);
  convertResourceReference(state.manager, level.gridVertexShader);
  convertResourceReference(state.manager, level.gridFragmentShader);
  convertResourceReference(state.manager, level.texture);

  dropShader(level.gridVertexShader.resource, state.engine);
  dropShader(level.gridFragmentShader.resource, state.engine);
}

void prepareLevel(Level& level, State& state) {
  std::vector<_GridInstanceData> data(MAX_GRID_SIZE * MAX_GRID_SIZE);

  F32x2 windowDim = windowDimensions(state.window);
  float tileRenderScale = (std::min(windowDim.x, windowDim.y) - 150.0f) /
                          static_cast<float>(MAX_GRID_SIZE);
  F32x2 offset = F32x2(60.0f);

  for (uint32_t i = 0; i < MAX_GRID_SIZE * MAX_GRID_SIZE; i++) {
    _GridInstanceData instance;
    Entity entity = level.gridEntities[i];
    SpriteAtlasRenderables renderable =
        level.registry.getComponent<SpriteAtlasRenderables>(entity);

    AtlasIndex index = textureAtlasIndex(I32x2(256), I32x2(32),
                                         static_cast<int>(renderable.type));
    uint32_t x = i % MAX_GRID_SIZE;
    uint32_t y = i / MAX_GRID_SIZE;
    F32x2 tilePosition = F32x2(x * tileRenderScale, y * tileRenderScale);

    instance.position = offset + tilePosition;
    instance.size = F32x2(tileRenderScale);
    instance.texturePosition = index.translation;
    instance.textureSize = index.scale;
    instance.rotation = renderable.rotation;
    data[i] = std::move(instance);
  }

  setBuffer(level.gridStorageBuffer.resource, 0, data.data(),
            data.size() * sizeof(_GridInstanceData));
}

void renderLevel(Level& level, State& state) {
  cmdBindPipeline(state.context, level.gridPipeline.resource);
  cmdBindVertexBuffer(state.context, state.rectVertexBuffer.resource);
  cmdBindIndexBuffer(state.context, state.rectIndexBuffer.resource);
  cmdBindDescriptorSet(state.context, level.gridDescriptorSet.resource,
                       level.gridPipeline.resource);
  cmdWritePushConstants(state.context, &state.perspective, sizeof(Perspective),
                        0, ShaderStage::VERTEX, level.gridPipeline.resource);
  cmdDrawIndexed(state.context, 6, MAX_GRID_SIZE * MAX_GRID_SIZE);
}

void dropLevel(Level& level, State& state) {
  dropTexture(level.texture.resource, state.engine);
  dropBuffer(level.gridStorageBuffer.resource, state.engine);
  dropDescriptorLayout(level.gridDescriptorLayout.resource, state.engine);
  dropPipeline(level.gridPipeline.resource, state.engine);
}
}  // namespace TDGame
