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
      F32x2::floor(F32x2(point.current) * world.icellDimensions);
  Cell& cell = world.cells[gridPos.x + gridPos.y * world.gridWidth];
  cell.pointIndices[cell.pointCount++] = world.size;

  ++world.size;
}

void updatePoint(Point& point, float dt) {
  // TODO: flag when velocity is too dramatic
  F32x2 vel = point.current - point.previous;

  // Air friciton approximation
  // TODO: move constant elsewhere? somewhere easily visible
  // TODO: investigate effect on scale
  // TODO: much better ways of implementing this?
  float const VELOCITY_DAMPING = 4.0f;

  point.previous = point.current;
  // NOTE: no delta time on velocity, since we already adjusted for that
  point.current = point.current + vel +
                  (point.acceleration - vel * VELOCITY_DAMPING) * (dt * dt);
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

World createWorld(F32x2 worldCenter, F32x2 worldDim, uint16_t gridWidth,
                  uint16_t gridHeight) {
  World world;
  world.gravity = F32x2(0.0f, -0.5f);
  world.center = worldCenter;
  world.dim = worldDim;

  world.gridHeight = gridHeight;
  world.gridWidth = gridWidth;

  world.size = 0;
  world.cellDimensions = F32x2(1.0f) / F32x2(gridWidth, gridHeight);
  world.icellDimensions = F32x2(1.0f) / world.cellDimensions;
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

void record(World& world, std::string const& imagePath) {
  world.recordedPoints = world.points;

  Image img = loadImage(imagePath.c_str(), TextureFormat::RGBA);

  // Basing the extent of the world on the position of points within
  //  alternatively use the world dimensions
  // TODO: get an actual maximum
  F32x2 worldMin = F32x2::inf();
  F32x2 worldMax = -F32x2::inf();
  F32x2 worldExtent = F32x2(0.0);

  for (Point const& p : world.points) {
    worldMin.x = std::min(worldMin.x, p.current.x);
    worldMin.y = std::min(worldMin.y, p.current.y);

    worldMax.x = std::max(worldMax.x, p.current.x);
    worldMax.y = std::max(worldMax.y, p.current.y);
  }

  // TODO: take floor or something instead
  worldExtent = worldMax - worldMin;

  std::vector<ColorAlpha> assignedColor =
      std::vector<ColorAlpha>(world.points.size());

  for (int i = 0; i < world.points.size(); i++) {
    F32x2 pos = world.points[i].current;

    // Should scale to 0-1
    pos = pos * F32x2(1.0f) / worldExtent;
    // Re-project to image dimensions
    // TODO: this doesn't consider the difference in dimensions
    pos *= F32x2(img.size);

    I32x2 imgCoords = I32x2(F32x2::floor(pos));
    int imagePixel = img.size.x * imgCoords.y + imgCoords.x;
    int const stride = 4;
    uint8_t r = imagePixel * stride + 0;
    uint8_t g = imagePixel * stride + 1;
    uint8_t b = imagePixel * stride + 2;
    uint8_t a = imagePixel * stride + 3;

    assignedColor[i] = ColorAlpha(r, g, b, a);
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
            F32x2::floor(F32x2(point.current) * world.icellDimensions);

        if (gridPos.x < 0 || gridPos.y < 0 || gridPos.x >= world.gridWidth ||
            gridPos.y >= world.gridHeight) {
          VIVIUM_LOG(LogSeverity::ERROR,
                     "Object at {} {} attempted to go to grid cell {} {} (OOB)",
                     point.current.x, point.current.y, gridPos.x, gridPos.y);
          continue;
        }

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
  // TODO: flag when energy transfer is too high
  const float restitution = 0.8f;
  const float epsilon = 0.0001f;

  F32x2 v = a.current - b.current;
  float distSquared = F32x2::dot(v, v);

  float radiusSum = a.radius + b.radius;
  float radiusSumSquared = radiusSum * radiusSum;

  // TODO: customised epsilon
  if (distSquared < radiusSumSquared && distSquared > epsilon) {
    // TODO: use vector norm so easier to optimise for compiler
    float dist = std::sqrt(distSquared);
    F32x2 normalised = v / dist;

    float aMassRatio = a.radius / radiusSum;
    float bMassRatio = b.radius / radiusSum;

    float delta = restitution * 0.5f * (dist - radiusSum);

    normalised *= delta;

    a.current -= bMassRatio * normalised;
    b.current += aMassRatio * normalised;
  }
}

void updatePosition(World& world, float dt) {
  for (uint64_t i = 0; i < world.size; i++) {
    updatePoint(world.points[i], dt);
  }
}

void update(World& world) {
  const int physicsSubsteps = 6;
  // 60fps rendering rate
  const float timePerFrame = 0.001f;
  const float substepDt = timePerFrame / physicsSubsteps;

  for (int i = 0; i < physicsSubsteps; i++) {
    updateCells(world);
    collisionResolution(world);
    applyGravity(world);
    updatePosition(world, substepDt);
    updateConstraints(world, substepDt);
    runConstraints(world, substepDt);
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

  convertResourceReference(state.manager, state.indexBuffer);
  convertResourceReference(state.manager, state.vertexBuffer);

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

void _update(State& state) { update(state.world); }

void _draw(State& state) {
  Perspective perspective = orthogonalPerspective2D(
      windowDimensions(state.window), F32x2(0.0f), 0.0f, 1.0f);

  // Draw verlet objects
  // Write render data to storage buffer
  std::vector<_PointInstanceData> points;
  points.reserve(state.world.size);

  F32x2 worldScale = state.world.dim;
  F32x2 worldCenter = state.world.center;
  float minimumScaleWorld = std::min(worldCenter.x, worldCenter.y);

  for (uint64_t i = 0; i < state.world.size; i++) {
    Point const& point = state.world.points[i];

    _PointInstanceData instance;
    instance.color = point.color;
    // TODO: this part doesnt make mathematical sense, or does it?
    instance.position =
        (point.current - F32x2(0.5f)) * minimumScaleWorld + worldCenter;
    instance.scale = F32x2(point.radius) * minimumScaleWorld;
    instance.normalVel = F32x2::normalise(point.current - point.previous);

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

void init(State& state) {
  _logInit();
  _fontInit();

  state.engine = createEngine(EngineOptions{});
  state.window = createWindow(WindowOptions{}, state.engine);
  state.manager = createManager();
  state.context = createCommandContext(state.engine);

  // TODO: optimise dimensions of grid, something like 2x radius of ball?
  // TODO: window dimensions dynamically change
  // NOTE: correct solution
  // - just keep all balls in the space of 0-1
  // - radius also a percentage of this space
  // - project the space up to the appropriate size of the screen in the
  // rendering code
  state.world = createWorld(F32x2(0.0), F32x2(1.0, 1.0), 60, 60);
  // Create a circle constraint
  CircleConstraint circle =
      createCircleConstraint(F32x2(0.5f), 0.4f, F32x2(0.0f), 0.0f, 0.0f, true);
  addConstraint(state.world, circle);

  // TODO: move the code, should be done in some initialisation function
  // Generate the font if it doesn't exist
  if (!std::filesystem::exists("vivium4/res/fonts/consola.sdf")) {
    compileSignedDistanceField("vivium4/res/fonts/consola.ttf", 512,
                               "vivium4/res/fonts/consola.sdf", 48, 1.0f);
  }

  state.font = createFontDistanceField("vivium4/res/fonts/consola.sdf");

  Input::init(state.window);

  _submit(state);
  allocateManager(state.manager, state.engine);

  _setup(state);
  clearManagerReferences(state.manager);
}

void run(State& state) {
  int ticks = 0;

  while (windowIsOpen(state.window, state.engine)) {
    engineBeginFrame(state.engine, state.context);

    Input::update(state.window);

    state.world.dim = F32x2(windowDimensions(state.window));
    state.world.center = F32x2(windowDimensions(state.window)) * 0.5f;

    // TODO: move to the update function?
    // Spawn in a ball
    if ((Input::get(Input::BTN_LEFT).state == Input::State::DOWN) &&
        state.world.size < 800) {
      // TODO: initial velocity
      Color c = Color(0.0f, 0.0f, 0.0f);
      c.b = randomFloat();
      c.g = randomFloat();
      c.r = randomFloat();

      float maxRadius = 0.05f;
      float radius = randomFloat(0.03f, maxRadius);

      Point newPoint = createPoint(F32x2(0.5f, 0.5f), radius, c);
      newPoint.current += randomVectorCirucmference(maxRadius);
      // TODO: big typo..
      newPoint.previous = newPoint.current + randomVectorCircle(0.01f);

      addPointToWorld(state.world, newPoint);
    }

    _update(state);

    windowBeginFrame(state.window, state.context, state.engine);
    windowBeginRender(state.window);

    _draw(state);

    windowEndRender(state.window);
    windowEndFrame(state.window, state.engine);

    ++ticks;

    engineEndFrame(state.engine);
  }

  VIVIUM_LOG(LogSeverity::DEBUG, "Window is closing now");
}

F32x2 approximateContactVelocity(QuadConstraint const& constraint,
                                 F32x2 currentPoint, float dt) {
  // TODO: should validate that the transformations we do here are correct
  // Calculate vector from point to constraint center
  F32x2 v = currentPoint - constraint.center;
  // Calculate change in angle
  // NOTE: this assumes we have constant angular velocity, consistent with an
  // object of infinite mass
  // Going back in time, hence minus
  float deltaAngle = -(constraint.angularVelocity * dt);
  // Use angle to calculate previous position, by rotating vector v around
  // origin
  // cos, -sin
  // sin, cos
  float oldX = v.x * std::cos(deltaAngle) - v.y * std::sin(deltaAngle);
  float oldY = v.y * std::sin(deltaAngle) + v.y * std::cos(deltaAngle);

  // Additionally consider constraint's velocity
  F32x2 previousPosition = F32x2(oldX, oldY);
  // Note again, negative since going back
  previousPosition += -constraint.velocity;

  // NOTE: this is the previous position of the constraint, we can use this to
  // approximately compute the velocity in verlet integration, we don't multiply
  // velocity by dt, we assume that's already factored in
  return previousPosition;
}

F32x2 approximateContactVelocity(CircleConstraint const& constraint,
                                 F32x2 currentPoint, float dt) {
  return constraint.center - constraint.velocity;
}

QuadConstraint createQuadConstraint(F32x2 pos, F32x2 dim, F32x2 velocity,
                                    float initialAngle, float angularVelocity,
                                    bool keepInside) {
  QuadConstraint c;
  c.velocity = velocity;
  c.center = pos;
  c.dimensions = dim;
  c.angle = initialAngle;
  c.angularVelocity = angularVelocity;
  c.keepInside = keepInside;

  return c;
}

CircleConstraint createCircleConstraint(F32x2 pos, float radius, F32x2 velocity,
                                        float initialAngle,
                                        float angularVelocity,
                                        bool keepInside) {
  CircleConstraint c;
  c.velocity = velocity;
  c.center = pos;
  c.radius = radius;
  c.angle = initialAngle;
  c.angularVelocity = angularVelocity;
  c.keepInside = keepInside;

  return c;
}

void addConstraint(World& world, QuadConstraint const& constraint) {
  world.quadConstraints.push_back(constraint);
}

void addConstraint(World& world, CircleConstraint const& constraint) {
  world.circleConstraints.push_back(constraint);
}

void updateConstraint(World& world, QuadConstraint& constraint, float dt) {
  constraint.angle += constraint.angularVelocity * dt;
  constraint.center += constraint.velocity;
}

void updateConstraint(World& world, CircleConstraint& constraint, float dt) {
  constraint.angle += constraint.angularVelocity * dt;
  constraint.center += constraint.velocity;
}

void considerConstraintCollisions(World& world, Point& point, float dt) {
  for (QuadConstraint const& quad : world.quadConstraints) {
    considerQuadConstraintCollision(quad, point, dt);
  }

  for (CircleConstraint const& circle : world.circleConstraints) {
    considerCircleConstraintCollision(circle, point, dt);
  }
}

// TODO: need to do some extra math given the approximate contact velocity
// Compute normal to the contact velocity?
void considerQuadConstraintCollision(QuadConstraint const& constraint,
                                     Point& point, float dt) {
  F32x2 constraintVelocity =
      approximateContactVelocity(constraint, point.current, dt);
  // Take normal
  // TODO: non trivial, need odwards-facing normal, should be towards the object
  // easy but poor solution: just compute both normals and take dot product,
  // take min dot product

  // 1. transform both objects to centerpoint
  F32x2 quadCenter = constraint.center;
  // The position of the point relative to the centerpoint of the quad
  F32x2 pointRelativeToQuad = point.current - constraint.center;

  // TODO: near phase collision check
  // TODO: transform the point such that it is in the same space as the rotated
  // square
  Mat2x2 quadRotation = Mat2x2::fromAngle(constraint.angle);
  F32x2 pointInQuadSpace = quadRotation * pointRelativeToQuad;

  F32x2 halfDim = constraint.dimensions * 0.5;

  // Now detect whether the point is in the correct side
  bool pointInConstraint = pointInAABB(pointInQuadSpace, -halfDim, halfDim);

  // TODO: compute all outwards-facing normals
  // TODO: only store edges, compute everything else with left/right rotations;
  // note clockwise direction
  std::vector<F32x2> const outwardsNormals = std::vector<F32x2>(
      {F32x2(-1, 0), F32x2(0, 1), F32x2(1, 0), F32x2(0, -1)});
  std::vector<F32x2> const edgeVectors = std::vector<F32x2>(
      {F32x2(0, 1), F32x2(1, 0), F32x2(0, -1), F32x2(-1, 0)});

  // TODO: save computation, if this point is not interesting to us given our
  // constraint,then just leave here

  // TODO: find closest edge here
  int closestEdgeIndex = -1;
  // TODO: note distance sqaured
  float closestEdgeDistance = std::numeric_limits<float>::max();

  for (int i = 0; i < outwardsNormals.size(); i++) {
    F32x2 normal = outwardsNormals[i];
    F32x2 edge = edgeVectors[i];
    // TODO: POTENTIALLY WRONG,have no clue about this maths just made it up
    F32x2 closestPoint = pointInQuadSpace * F32x2::dot(pointInQuadSpace, edge);
    F32x2 closestVector = pointInQuadSpace - closestPoint;
    float distanceSquared = F32x2::dot(closestVector, closestVector);

    if (distanceSquared < closestEdgeDistance) {
      closestEdgeDistance = distanceSquared;
      closestEdgeIndex = i;
    }
  }

  // TODO: validate closestEdgeIndex,should just be an assertion
  if (closestEdgeIndex == -1) {
    VIVIUM_LOG(LogSeverity::FATAL, "Closest edge index was -1, impossibility");
  }

  F32x2 selectedNormal;

  // TODO: for these functions, just select the correct direction of normals,
  // and the correct one as well? that should be sufficient to keep the rest of
  // the code not in a branhc
  if (constraint.keepInside && !pointInConstraint) {
    selectedNormal = -outwardsNormals[closestEdgeIndex];
    // TODO: we should move this point back into the constraint;
    // for this, select the correct normals
  } else if (!constraint.keepInside && pointInConstraint) {
    // TODO: select outwards facing normals
    selectedNormal = outwardsNormals[closestEdgeIndex];
  } else {
    return;
  }

  // TODO: consider margin additioanlly
  // TODO: validate correctness of computation
  pointInQuadSpace =
      pointInQuadSpace +
      selectedNormal * (std::sqrt(closestEdgeDistance) + point.radius);

  Mat2x2 inverseRotation = quadRotation.transpose();
  F32x2 untransformedPoint = inverseRotation * pointInQuadSpace;
  // TODO: unsure if correct
  untransformedPoint += constraint.center;

  // TODO: current, or previous?
  point.current = untransformedPoint;

  // TODO: implementation of moving along that vector
  // TODO: rotation out of quad space
  // TODO: set old/new position to whatever it should be?

  // Calculate every edge of the quad
  // If the point lies within, or outside our shape (depending on keepInside)
  // we consider it

  // Given distance to the nearest edge
  // 1. we set the ball to the closest point to that edge, and output the ball
  // (inside, or outside), along the correct normal as to push it in the
  // constraint's desired direction
}

void considerCircleConstraintCollision(CircleConstraint const& constraint,
                                       Point& point, float dt) {
  F32x2 delta = point.current - constraint.center;
  float dist = F32x2::length(delta);
  float radiusSum = constraint.radius + point.radius;

  if (dist == 0.0f) return;

  F32x2 normal = delta / dist;

  const float margin = 0.001f;

  // outside when should be inside
  if (constraint.keepInside && dist > radiusSum) {
    float penetration = dist - radiusSum;
    point.current -= penetration * normal + F32x2(margin);
  }

  // inside when should be outside
  // TODO: check maths on this
  else if (!constraint.keepInside && dist < radiusSum) {
    float penetration = radiusSum - dist;
    point.current = constraint.center + penetration * normal + F32x2(margin);
  }
}

void updateConstraints(World& world, float dt) {
  for (CircleConstraint& c : world.circleConstraints) {
    updateConstraint(world, c, dt);
  }

  for (QuadConstraint& c : world.quadConstraints) {
    updateConstraint(world, c, dt);
  }
}

void runConstraints(World& world, float dt) {
  for (Point& point : world.points) {
    for (CircleConstraint const& c : world.circleConstraints) {
      considerCircleConstraintCollision(c, point, dt);
    }

    for (QuadConstraint const& c : world.quadConstraints) {
      considerQuadConstraintCollision(c, point, dt);
    }
  }
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
