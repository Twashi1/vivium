#include "state.h"

#include "grid.h"

namespace TDGame {
void initialise(State& state) {
  _logInit();
  _fontInit();

  state.engine = createEngine(EngineOptions{});
  state.window = createWindow(WindowOptions{}, state.engine);

  // TODO: shouldn't be forced to provide this if we don't use the GUI to render
  // sprites
  state.guiAtlas.data = nullptr;
  state.guiAtlas.size = I32x2(0);
  state.guiAtlas.format = TextureFormat::RGBA;

  state.context = createCommandContext(state.engine);
  state.manager = createManager();
  state.guiContext = createGUIContext(state.manager, state.engine, state.window,
                                      &state.guiAtlas);

  submitState(state);

  allocateManager(state.manager, state.engine);

  setupState(state);
  setupGUIContext(state.guiContext, state.manager, state.context, state.engine);

  clearManagerReferences(state.manager);
}

void gameloop(State& state) {
  state.perspective = orthogonalPerspective2D(windowDimensions(state.window),
                                              F32x2(0.0f), 0.0f, 1.0f);

  while (windowIsOpen(state.window, state.engine)) {
    engineBeginFrame(state.engine, state.context);

    prepareLevel(state.level, state);

    windowBeginFrame(state.window, state.context, state.engine);
    windowBeginRender(state.window);

    state.perspective = orthogonalPerspective2D(windowDimensions(state.window),
                                                F32x2(0.0f), 0.0f, 1.0f);
    renderLevel(state.level, state);

    windowEndRender(state.window);
    windowEndFrame(state.window, state.engine);

    engineEndFrame(state.engine);
  }
}

void terminate(State& state) {
  dropState(state);

  dropGUIContext(state.guiContext, state.engine);
  dropCommandContext(state.context, state.engine);
  dropManager(state.manager, state.engine);
  dropWindow(state.window, state.engine);
  dropEngine(state.engine);
}

void submitState(State& state) {
  // Rect buffers
  submitResource(
      state.manager, &state.rectVertexBuffer.reference, MemoryType::DEVICE,
      std::vector<BufferSpecification>({
          BufferSpecification(8 * sizeof(F32x2), BufferUsage::VERTEX),
      }));
  submitResource(
      state.manager, &state.rectIndexBuffer.reference, MemoryType::DEVICE,
      std::vector<BufferSpecification>({
          BufferSpecification(6 * sizeof(uint16_t), BufferUsage::INDEX),
      }));

  submitLevel(state.level, state);
}

void setupState(State& state) {
  convertResourceReference(state.manager, state.rectVertexBuffer);
  convertResourceReference(state.manager, state.rectIndexBuffer);

  float vertexData[] = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f};
  uint16_t indexData[] = {0, 1, 2, 2, 3, 0};

  VkDeviceMemory temporaryMemory;
  VkBuffer stagingBuffer;
  void* stagingMapping;
  // Staging buffer both for vertex data and index data
  _cmdCreateTransientStagingBuffer(
      state.engine, &stagingBuffer, &temporaryMemory,
      16 * sizeof(float) + 6 * sizeof(uint16_t), &stagingMapping);

  Buffer resource;
  resource.buffer = stagingBuffer;
  resource.mapping = stagingMapping;

  contextBeginTransfer(state.context);

  memcpy(stagingMapping, vertexData, 16 * sizeof(float));
  cmdTransferBuffer(state.context, resource, 16 * sizeof(float), 0,
                    state.rectVertexBuffer.resource);

  memcpy(reinterpret_cast<uint8_t*>(stagingMapping) + 16 * sizeof(float),
         indexData, 6 * sizeof(uint16_t));
  cmdTransferBuffer(state.context, resource, 6 * sizeof(uint16_t),
                    16 * sizeof(float), state.rectIndexBuffer.resource);

  contextEndTransfer(state.context, state.engine);

  _cmdFreeTransientStagingBuffer(state.engine, stagingBuffer, temporaryMemory);

  setupLevel(state.level, state);
}

void dropState(State& state) {
  dropBuffer(state.rectVertexBuffer.resource, state.engine);
  dropBuffer(state.rectIndexBuffer.resource, state.engine);

  dropLevel(state.level, state);
}
};  // namespace TDGame
