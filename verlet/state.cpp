#include "state.h"

namespace Verlet {
Point createPoint(F32x2 pos, float radius, Color color) {
  Point p;
  p.current = pos;
  p.previous = pos;
  p.radius = radius;
  p.color = color;
  p.acceleration = F32x2(0.0f);

  return p;
}

void updatePoint(Point& point, float dt) {
  F32x2 vel = point.current - point.previous;

  point.previous = point.current;
  // NOTE: no delta time on velocity, since we already adjusted for that
  point.current = point.current + vel + point.acceleration * (dt * dt);
  point.acceleration = F32x2(0.0f);
}

Cell createCell() {
  Cell cell;
  cell.pointCount = 0;
  cell.pointIndices = new uint16_t[MAX_CELL_OBJECTS];

  return cell;
}

void dropCell(Cell& cell) {
  if (cell.pointIndices == nullptr) return;
  delete[] cell.pointIndices;
}

void swapRemoveFromCell(Cell& cell, uint16_t index) {
  cell.pointIndices[index] = cell.pointIndices[--cell.pointCount];
}

World createWorld(F32x2 screenDim, uint16_t gridWidth, uint16_t gridHeight) {
  World world;
  world.gravity = F32x2(0.0f, -450.0f);
  world.center = F32x2(400.0f, 300.0f);
  world.dim = F32x2(0.0f);
  world.radius = 200.0f;

  world.gridHeight = gridHeight;
  world.gridWidth = gridWidth;

  world.size = 0;
  world.gridDimensions = screenDim / F32x2(gridWidth, gridHeight);
  world.igridDimensions = F32x2(1.0f) / world.gridDimensions;
  world.resolutionCounter = 0;

  world.points = std::vector<Point>(MAX_VERLET_OBJECTS);
  world.cells = std::vector<Cell>(gridWidth * gridHeight);

  for (uint64_t i = 0; i < gridWidth * gridHeight; i++) {
    world.cells[i] = createCell();
  }

  return world;
}

void dropWorld(World& world) {
  for (Cell& cell : world.cells) {
    dropCell(cell);
  }
}

void updateCells(World& world) {
  for (uint32_t gy = 0; gy < world.gridHeight; gy++) {
    for (uint32_t gx = 0; gx < world.gridWidth; gx++) {
      Cell& cell = world.cells[gx + gy * world.gridWidth];

      for (uint64_t i = 0; i < cell.pointCount; i++) {
        uint16_t objIndex = cell.pointIndices[i];

        Point& point = world.points[objIndex];

        Vec2<uint16_t> gridPos =
            F32x2::floor(F32x2(point.current) * world.igridDimensions);

        if (gridPos.x == gx && gridPos.y == gy) {
          continue;
        }

        // Remove from old cell
        swapRemoveFromCell(cell, i);

        // Add to new
        Cell& targetCell = world.cells[gridPos.x + gridPos.y * world.gridWidth];
        targetCell.pointIndices[targetCell.pointCount++] = objIndex;
      }
    }
  }
}

void applyGravity(World& world) {
  for (uint64_t i = 0; i < world.size; i++) {
    world.points[i].acceleration += world.gravity;
  }
}

void constraintSquare(World& world) {
  float left = world.center.x - world.dim.x;
  float right = world.center.x + world.dim.x;
  float bot = world.center.y - world.dim.y;
  float top = world.center.y + world.dim.y;

  for (uint64_t i = 0; i < world.size; i++) {
    Point& point = world.points[i];

    if (point.current.x - point.radius < left) {
      point.current.x = left + point.radius;
    } else if (point.current.x + point.radius > right) {
      point.current.x = right - point.radius;
    }

    if (point.current.y - point.radius < bot) {
      point.current.y = bot + point.radius;
    } else if (point.current.y + point.radius > top) {
      point.current.y = top - point.radius;
    }
  }
}

void collisionResolution(World& world) {
  // TODO: implementation
}

void updatePosition(World& world, float dt) {
  for (uint64_t i = 0; i < world.size; i++) {
    updatePoint(world.points[i], dt);
  }
}

void update(World& world) {
  const int physicsSubsteps = 4;
  // 60fps rendering rate
  const float timePerFrame = 0.016f;
  const float substepDt = timePerFrame / physicsSubsteps;

  updateCells(world);

  for (int i = 0; i < physicsSubsteps; i++) {
    applyGravity(world);
    constraintSquare(world);
    collisionResolution(world);
    updatePosition(world, substepDt);
  }
}

void _submit(State& state) {}

void _setup(State& state) {}

void _update(State& state) {}

void _draw(State& state) {
  Perspective perspective = orthogonalPerspective2D(
      windowDimensions(state.window), F32x2(0.0f), 0.0f, 1.0f);

  // for (PipelineInstance& instance : state.pipelineInstances) {
  //   cmdBindPipeline(state.context, instance.pipeline.resource);
  //   cmdBindVertexBuffer(state.context, instance.vertexBuffer.resource);
  //   cmdBindIndexBuffer(state.context, instance.indexBuffer.resource);
  //   cmdBindDescriptorSet(state.context, instance.descriptor.resource,
  //                        instance.pipeline.resource);
  //   cmdWritePushConstants(state.context, &perspective, sizeof(Perspective),
  //   0,
  //                         ShaderStage::VERTEX, instance.pipeline.resource);
  //   cmdDrawIndexed(state.context, instance.indexCount,
  //   instance.instanceCount);
  // }
}

void init(State& state, std::string bytecodeFilename) {
  _logInit();
  _fontInit();

  state.engine = createEngine(EngineOptions{});
  state.window = createWindow(WindowOptions{}, state.engine);
  state.manager = createManager();
  state.context = createCommandContext(state.engine);

  Input::init(state.window);

  _submit(state);
  allocateManager(state.manager, state.engine);

  _setup(state);
  clearManagerReferences(state.manager);
}

void run(State& state) {
  while (windowIsOpen(state.window, state.engine)) {
    engineBeginFrame(state.engine, state.context);

    Input::update(state.window);

    _update(state);

    windowBeginFrame(state.window, state.context, state.engine);
    windowBeginRender(state.window);

    _draw(state);

    windowEndRender(state.window);
    windowEndFrame(state.window, state.engine);

    engineEndFrame(state.engine);
  }

  VIVIUM_LOG(LogSeverity::DEBUG, "Window is closing now");
}

void drop(State& state) {
  dropManager(state.manager, state.engine);
  dropCommandContext(state.context, state.engine);

  dropWindow(state.window, state.engine);
  dropEngine(state.engine);

  _fontTerminate();
}
}  // namespace Verlet
