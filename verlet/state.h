#pragma once

#include "../vivium4/vivium4.h"

using namespace Vivium;

constexpr uint64_t MAX_VERLET_OBJECTS = 8000;
constexpr uint64_t MAX_CELL_OBJECTS = 800;

namespace Verlet {
struct _PointInstanceData {
  F32x2 position;
  F32x2 scale;
  Color color;
  float _fill0;  // TODO: use directives for alignas, and size
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

struct World {
  F32x2 gravity;
  F32x2 center;
  F32x2 dim;
  float radius;

  uint64_t size;
  std::vector<Point> points;
  std::vector<Cell> cells;
  uint32_t gridWidth;  // TODO: difference between these and grid dimensions?
  uint32_t gridHeight;

  F32x2 gridDimensions;
  F32x2 igridDimensions;

  uint64_t resolutionCounter;

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

Cell createCell();
// NOTE: this is not the index of the point within the global point list, but
// within the cell we have no other way to identify a point within the cell in
// constant time
void swapRemoveFromCell(Cell& cell, uint16_t index);
void dropCell(Cell& cell);

World createWorld();
void dropWorld(World& world);

void addPointToWorld(World& world, Point point);

void updateCells(World& world);
void update(World& world);

void applyGravity(World& world);
void constraintSquare(World& world);
void collisionResolution(World& world);
void updatePosition(World& world, float dt);

void resolveCollision(Point& a, Point& b);
void resolveCells(World& world, Cell& cellA, Cell& cellB);

void init(State& state);
void run(State& state);
void drop(State& state);

void _submit(State& state);
void _setup(State& state);
void _update(State& state);
void _draw(State& state);
void _drop(State& state);
}  // namespace Verlet
