#include "state.h"

namespace Verlet {
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
