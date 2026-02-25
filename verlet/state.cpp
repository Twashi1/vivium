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

void addPointToWorld(World& world, Point point) {
  // TODO: maybe some logs
  if (world.size >= MAX_VERLET_OBJECTS) return;

  world.points[world.size] = point;
  // TODO: initiate with random velocity
  Vec2<uint16_t> gridPos =
      F32x2::floor(F32x2(point.current) * world.igridDimensions);
  Cell& cell = world.cells[gridPos.x + gridPos.y * world.gridWidth];
  cell.pointIndices[cell.pointCount++] = world.size;

  ++world.size;
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
  for (uint16_t gy = 0; gy < world.gridHeight; gy++) {
    // Past thomas did this in a nice way
    uint16_t miny = (gy == 0) ? 0 : gy - 1;
    uint16_t maxy =
        (gy == world.gridHeight - 1) ? world.gridHeight - 1 : gy + 1;

    for (uint16_t gx = 0; gx < world.gridWidth; gx++) {
      Cell& cellA = world.cells[gx + gy * world.gridWidth];

      uint16_t minx = (gx == 0) ? 0 : gx - 1;
      uint16_t maxx =
          (gx == world.gridWidth - 1) ? world.gridWidth - 1 : gx + 1;

      for (uint16_t ny = miny; ny <= maxy; ny++) {
        for (uint16_t nx = minx; nx <= maxx; nx++) {
          Cell& cellB = world.cells[nx + ny * world.gridWidth];

          resolveCells(world, cellA, cellB);
        }
      }
    }
  }
}

void resolveCells(World& world, Cell& cellA, Cell& cellB) {
  for (uint64_t i = 0; i < cellA.pointCount; i++) {
    uint16_t indexA = cellA.pointIndices[i];
    Point& pointA = world.points[indexA];

    for (uint64_t j = 0; j < cellB.pointCount; j++) {
      uint16_t indexB = cellB.pointIndices[j];

      // If we're referencing the same object
      if (indexA == indexB) continue;

      Point& pointB = world.points[indexB];

      resolveCollision(pointA, pointB);
    }
  }
}

void resolveCollision(Point& a, Point& b) {
  const float restitution = 0.65f;

  F32x2 v = a.current - b.current;
  float distSquared = F32x2::dot(v, v);

  float radiusSum = a.radius + b.radius;
  float radiusSumSquared = radiusSum * radiusSum;

  // TODO: customised epsilon
  if (distSquared < radiusSumSquared && distSquared > 0.01f) {
    // TODO: use vector norm so easier to optimise for compiler
    float dist = std::sqrt(distSquared);
    F32x2 normalised = v / dist;

    float aMassRatio = a.radius / radiusSum;
    float bMassRatio = b.radius / radiusSum;

    float delta = 0.5f * (dist - radiusSum);

    normalised *= delta * restitution;

    a.current -= bMassRatio * normalised;
    b.current += bMassRatio * normalised;
  }
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

void _submit(State& state) {
  // Vertex/index buffers
  std::array<BufferReference, 2> deviceBuffers;

  submitResource(
      state.manager, deviceBuffers.data(), MemoryType::DEVICE,
      std::vector<BufferSpecification>(
          {BufferSpecification(4 * sizeof(F32x2), BufferUsage::VERTEX),
           BufferSpecification(6 * sizeof(uint16_t), BufferUsage::INDEX)}));

  state.vertexBuffer.reference = deviceBuffers[0];
  state.indexBuffer.reference = deviceBuffers[1];

  // Text
  submitResource(
      state.manager, &state.fontTexture.reference,
      std::vector<TextureSpecification>({TextureSpecification::fromFont(
          state.font, TextureFormat::MONOCHROME, TextureFilter::NEAREST)}));

  state.text.bufferLayout = BufferLayout::fromTypes(std::vector<ShaderDataType>(
      {ShaderDataType::VEC2, ShaderDataType::VEC2, ShaderDataType::VEC3}));

  submitResource(
      state.manager, &state.text.descriptorLayout.reference,
      std::vector<DescriptorLayoutSpecification>({DescriptorLayoutSpecification(
          std::vector<UniformBinding>({UniformBinding(
              ShaderStage::FRAGMENT, 0, UniformType::TEXTURE)}))}));

  submitResource(state.manager, &state.text.fragmentShader.reference,
                 std::vector<ShaderSpecification>({compileShader(
                     ShaderStage::FRAGMENT, "vivium4/res/text.frag",
                     "vivium4/res/text_frag.spv")}));

  submitResource(state.manager, &state.text.vertexShader.reference,
                 std::vector<ShaderSpecification>({compileShader(
                     ShaderStage::VERTEX, "vivium4/res/text.vert",
                     "vivium4/res/text_vert.spv")}));

  submitResource(
      state.manager, &state.text.pipeline.reference,
      std::vector<PipelineSpecification>({PipelineSpecification::fromWindow(
          std::vector<ShaderReference>({state.text.fragmentShader.reference,
                                        state.text.vertexShader.reference}),
          state.text.bufferLayout,
          std::vector<DescriptorLayoutReference>(
              {state.text.descriptorLayout.reference}),
          std::vector<PushConstant>(
              {PushConstant(ShaderStage::VERTEX, 0, sizeof(Perspective))}),
          state.window)}));

  // Verlet submit
  submitResource(
      state.manager, &state.points.storageBuffer.reference, MemoryType::UNIFORM,
      std::vector<BufferSpecification>(
          {BufferSpecification(MAX_VERLET_OBJECTS * sizeof(_PointInstanceData),
                               BufferUsage::STORAGE)}));

  submitResource(
      state.manager, &state.points.descriptorLayout.reference,
      std::vector<DescriptorLayoutSpecification>({DescriptorLayoutSpecification(
          std::vector<UniformBinding>({UniformBinding(
              ShaderStage::VERTEX, 0, UniformType::STORAGE_BUFFER)}))}));

  submitResource(
      state.manager, &state.points.descriptorSet.reference,
      std::vector<DescriptorSetSpecification>({DescriptorSetSpecification(
          state.points.descriptorLayout.reference,
          std::vector<UniformData>({UniformData::fromBuffer(
              state.points.storageBuffer.reference,
              MAX_VERLET_OBJECTS * sizeof(_PointInstanceData), 0)}))}));

  submitResource(state.manager, &state.points.fragmentShader.reference,
                 std::vector<ShaderSpecification>({compileShader(
                     ShaderStage::FRAGMENT, "verlet/res/verlet.frag",
                     "verlet/res/verlet_frag.spv")}));

  submitResource(state.manager, &state.points.vertexShader.reference,
                 std::vector<ShaderSpecification>({compileShader(
                     ShaderStage::VERTEX, "verlet/res/verlet.vert",
                     "verlet/res/verlet_vert.spv")}));

  submitResource(
      state.manager, &state.points.pipeline.reference,
      std::vector<PipelineSpecification>({PipelineSpecification::fromWindow(
          std::vector<ShaderReference>({state.points.fragmentShader.reference,
                                        state.points.vertexShader.reference}),
          BufferLayout::fromTypes(
              std::vector<ShaderDataType>({ShaderDataType::VEC2})),
          std::vector<DescriptorLayoutReference>(
              {state.points.descriptorLayout.reference}),
          std::vector<PushConstant>(
              {PushConstant(ShaderStage::VERTEX, 0, sizeof(Perspective))}),
          state.window)}));
}

void _setup(State& state) {
  convertResourceReference(state.manager, state.fontTexture);

  convertResourceReference(state.manager, state.text.pipeline);
  convertResourceReference(state.manager, state.text.descriptorLayout);
  convertResourceReference(state.manager, state.text.fragmentShader);
  convertResourceReference(state.manager, state.text.vertexShader);

  convertResourceReference(state.manager, state.points.vertexShader);
  convertResourceReference(state.manager, state.points.fragmentShader);
  convertResourceReference(state.manager, state.points.descriptorLayout);
  convertResourceReference(state.manager, state.points.descriptorSet);
  convertResourceReference(state.manager, state.points.pipeline);
  convertResourceReference(state.manager, state.points.storageBuffer);

  float vertexData[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
  uint16_t indexData[] = {0, 1, 2, 2, 3, 0};

  VkDeviceMemory temporaryMemory;
  VkBuffer stagingBuffer;
  void* stagingMapping;
  // Staging buffer both for vertex data and index data
  _cmdCreateTransientStagingBuffer(
      state.engine, &stagingBuffer, &temporaryMemory,
      8 * sizeof(float) + 6 * sizeof(uint16_t), &stagingMapping);

  Buffer resource;
  resource.buffer = stagingBuffer;
  resource.mapping = stagingMapping;

  contextBeginTransfer(state.context);

  memcpy(stagingMapping, vertexData, 8 * sizeof(float));
  cmdTransferBuffer(state.context, resource, 8 * sizeof(float), 0,
                    state.vertexBuffer.resource);

  memcpy(reinterpret_cast<uint8_t*>(stagingMapping) + 8 * sizeof(float),
         indexData, 6 * sizeof(uint16_t));
  cmdTransferBuffer(state.context, resource, 6 * sizeof(uint16_t),
                    8 * sizeof(float), state.indexBuffer.resource);

  contextEndTransfer(state.context, state.engine);

  _cmdFreeTransientStagingBuffer(state.engine, stagingBuffer, temporaryMemory);

  dropShader(state.text.fragmentShader.resource, state.engine);
  dropShader(state.text.vertexShader.resource, state.engine);

  dropShader(state.points.fragmentShader.resource, state.engine);
  dropShader(state.points.vertexShader.resource, state.engine);
}

void _update(State& state) {}

void _draw(State& state) {
  Perspective perspective = orthogonalPerspective2D(
      windowDimensions(state.window), F32x2(0.0f), 0.0f, 1.0f);

  // Draw verlet objects
  // Write render data to storage buffer
  std::vector<_PointInstanceData> points;
  points.reserve(state.world.size);

  for (uint64_t i = 0; i < state.world.size; i++) {
    Point const& point = state.world.points[i];

    _PointInstanceData instance;
    instance.color = point.color;
    instance.position = point.current;
    instance.scale = F32x2(point.radius);

    points.push_back(std::move(instance));
  }

  setBuffer(state.points.storageBuffer.resource, 0, points.data(),
            points.size() * sizeof(_PointInstanceData));

  cmdBindPipeline(state.context, state.points.pipeline.resource);
  cmdBindVertexBuffer(state.context, state.vertexBuffer.resource);
  cmdBindIndexBuffer(state.context, state.indexBuffer.resource);
  cmdBindDescriptorSet(state.context, state.points.descriptorSet.resource,
                       state.points.pipeline.resource);
  cmdWritePushConstants(state.context, &perspective, sizeof(Perspective), 0,
                        ShaderStage::VERTEX, state.points.pipeline.resource);
  // TODO: grab correct number of instances
  cmdDrawIndexed(state.context, 6, points.size());
}

void _drop(State& state) {
  dropTexture(state.fontTexture.resource, state.engine);

  dropDescriptorLayout(state.text.descriptorLayout.resource, state.engine);
  dropDescriptorLayout(state.points.descriptorLayout.resource, state.engine);

  dropPipeline(state.text.pipeline.resource, state.engine);
  dropPipeline(state.points.pipeline.resource, state.engine);

  dropBuffer(state.points.storageBuffer.resource, state.engine);
  dropBuffer(state.indexBuffer.resource, state.engine);
  dropBuffer(state.vertexBuffer.resource, state.engine);
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
  _drop(state);

  dropManager(state.manager, state.engine);
  dropCommandContext(state.context, state.engine);

  dropWindow(state.window, state.engine);
  dropEngine(state.engine);

  _fontTerminate();
}
}  // namespace Verlet
