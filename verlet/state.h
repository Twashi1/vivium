#pragma once

#include "../vivium4/vivium4.h"

using namespace Vivium;

constexpr uint64_t MAX_VERLET_OBJECTS = 8000;
constexpr uint64_t MAX_CELL_OBJECTS = 800;

namespace Verlet {
struct _PointInstanceData {
  F32x2 position;
  F32x2 scale;
  F32x2 normalVel;
  F32x2 _fill0;
  Color color;
  float _fill1;  // TODO: use directives for alignas, and size
};

struct Point {
  F32x2 current;
  F32x2 previous;
  F32x2 acceleration;

  float radius;
  Color color;
};

struct Cell {
  uint64_t pointCount;
  uint16_t* pointIndices;
};

struct QuadConstraint {
  F32x2 dimensions;
  F32x2 center;
  float angle;

  float angularVelocity;
  F32x2 velocity;

  // Do we attempt to keep objects inside this constraint, or outside
  bool keepInside;
};

struct CircleConstraint {
  float radius;
  F32x2 center;
  float angle;

  F32x2 velocity;
  float angularVelocity;

  // Do we attempt to keep objets inside this contraint, or outside
  bool keepInside;
};

struct World {
  F32x2 gravity;
  F32x2 center;
  F32x2 dim;

  uint64_t size;
  std::vector<Point> points;
  std::vector<Cell> cells;

  uint32_t gridWidth;  // TODO: difference between these and grid dimensions?
  uint32_t gridHeight;

  F32x2 cellDimensions;
  F32x2 icellDimensions;

  uint64_t resolutionCounter;

  std::vector<Point> recordedPoints;
  std::vector<Color> finalColors;
  std::vector<QuadConstraint> quadConstraints;
  std::vector<CircleConstraint> circleConstraints;

  Time::Timer timer;  // TODO: don't need this?
};

struct State {
  Engine engine;
  Window window;
  CommandContext context;
  ResourceManager manager;

  Ref<Buffer> vertexBuffer;
  Ref<Buffer> indexBuffer;

  Font font;
  Ref<Texture> fontTexture;

  struct {
    Ref<Pipeline> pipeline;
    Ref<DescriptorLayout> descriptorLayout;
    Ref<Shader> fragmentShader;
    Ref<Shader> vertexShader;

    // Batch buffer layout
    BufferLayout bufferLayout;

    std::vector<TextBatch*> texts;
  } text;

  struct {
    Ref<Shader> fragmentShader;
    Ref<Shader> vertexShader;

    Ref<Buffer> storageBuffer;
    Ref<DescriptorLayout> descriptorLayout;
    Ref<DescriptorSet> descriptorSet;
    Ref<Pipeline> pipeline;
  } points;

  World world;
};

Point createPoint(F32x2 pos, float radius, Color color);
void updatePoint(Point& point, float dt);

QuadConstraint createQuadConstraint(F32x2 pos, F32x2 dim, F32x2 velocity,
                                    float initialAngle, float angularVelocity,
                                    bool keepInside);
CircleConstraint createCircleConstraint(F32x2 pos, float radius, F32x2 velocity,
                                        float initialAngle,
                                        float angularVelocity, bool keepInside);

void addConstraint(World& world, QuadConstraint const& constraint);
void addConstraint(World& world, CircleConstraint const& constraint);

void updateConstraint(World& world, QuadConstraint& constraint, float dt);
void updateConstraint(World& world, CircleConstraint& constraint, float dt);

void considerConstraintCollisions(World& world, Point& point, float dt);
void considerQuadConstraintCollision(QuadConstraint const& constraint,
                                     Point& point, float dt);
void considerCircleConstraintCollision(CircleConstraint const& constraint,
                                       Point& point, float dt);
void updateConstraints(World& world, float dt);
void runConstraints(World& world, float dt);

Cell createCell();
// NOTE: this is not the index of the point within the global point list, but
// within the cell we have no other way to identify a point within the cell in
// constant time
void swapRemoveFromCell(Cell& cell, uint16_t index);
void dropCell(Cell& cell);

World createWorld(F32x2 worldCenter, F32x2 worldDim, uint16_t gridWidth,
                  uint16_t gridHeight);
void dropWorld(World& world);

void addPointToWorld(World& world, Point point);

void updateCells(World& world);
void update(World& world);

void applyGravity(World& world);
// TODO: remove this, deprecated by custom constraints``
void collisionResolution(World& world);
void updatePosition(World& world, float dt);

void resolveCollision(Point& a, Point& b);
void resolveCells(World& world, Cell& cellA, Cell& cellB);

void init(State& state);
void run(State& state);
void drop(State& state);

F32x2 approximateContactVelocity(QuadConstraint const& constraint,
                                 F32x2 currentPoint, float dt);
F32x2 approximateContactVelocity(CircleConstraint const& constraint,
                                 F32x2 currentPoint, float dt);

void record(World& world, std::string const& imgPath);

void _submit(State& state);
void _setup(State& state);
void _update(State& state);
void _draw(State& state);
void _drop(State& state);
}  // namespace Verlet
