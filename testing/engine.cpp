#include "engine.h"

namespace Testing {
void runEngineTest() {
  // TestSuite suite =
  //     createSuite("Engine suite", defaultFormatter(), consoleOutstream);
  // pushHeader(suite, "Engine creation");
  // endHeader(suite);
  // finishSuite(suite);
  // VIVIUM_LOG(LogSeverity::DEBUG, "Printed suite");

  _logInit();
  _fontInit();

  // Initialisation
  Engine engine = createEngine(EngineOptions{});
  Window window = createWindow(WindowOptions{}, engine);
  ResourceManager manager = createManager();
  CommandContext ctx = createCommandContext(engine);
  Input::init(window);
  randomSeed(0);

  if (!std::filesystem::exists("vivium4/res/fonts/consola.sdf")) {
    compileSignedDistanceField("vivium4/res/fonts/consola.ttf", 512,
                               "vivium4/res/fonts/consola.sdf", 48, 1.0f);
  }

  Font font = createFontDistanceField("vivium4/res/fonts/consola.sdf");

  // Submission

  std::array<BufferReference, 1> computeBuffers;
  submitResource(manager, computeBuffers.data(), MemoryType::UNIFORM,
                 std::vector<BufferSpecification>({BufferSpecification(
                     1024 * sizeof(float), BufferUsage::COMPUTE)}));
  Ref<Buffer> refComputeBuffer;
  refComputeBuffer.reference = computeBuffers[0];

  std::array<DescriptorLayoutReference, 1> descriptorLayout;
  submitResource(
      manager, descriptorLayout.data(),
      std::vector<DescriptorLayoutSpecification>({DescriptorLayoutSpecification(
          std::vector<UniformBinding>({UniformBinding(
              ShaderStage::COMPUTE, 0, UniformType::STORAGE_BUFFER)}))}));
  Ref<DescriptorLayout> refDescriptorLayout;
  refDescriptorLayout.reference = descriptorLayout[0];

  std::array<ShaderReference, 1> computeShader;
  submitResource(manager, computeShader.data(),
                 std::vector<ShaderSpecification>({{compileShader(
                     ShaderStage::COMPUTE, "vivium4/res/double.comp",
                     "vivium4/res/double_comp.spv")}}));
  Ref<Shader> refShader;
  refShader.reference = computeShader[0];

  std::array<PipelineReference, 1> pipeline;
  submitResource(
      manager, pipeline.data(),
      std::vector<PipelineSpecification>({PipelineSpecification::fromCompute(
          std::vector<ShaderReference>({computeShader[0]}),
          BufferLayout::fromTypes(
              {std::vector<ShaderDataType>({ShaderDataType::FLOAT})}),
          std::vector<DescriptorLayoutReference>({descriptorLayout[0]}),
          std::vector<PushConstant>({}))}));
  Ref<Pipeline> refPipeline;
  refPipeline.reference = pipeline[0];

  std::array<DescriptorSetReference, 1> descriptorSet;
  submitResource(
      manager, descriptorSet.data(),
      std::vector<DescriptorSetSpecification>({DescriptorSetSpecification(
          descriptorLayout[0],
          std::vector<UniformData>({UniformData::fromBuffer(
              computeBuffers[0], 1024 * sizeof(float), 0)}))}));
  Ref<DescriptorSet> refDescriptorSet;
  refDescriptorSet.reference = descriptorSet[0];

  VIVIUM_LOG(LogSeverity::DEBUG, "About to allocate..");

  allocateManager(manager, engine);
  VIVIUM_LOG(LogSeverity::DEBUG, "Allocated..");

  // Setup
  convertResourceReference(manager, refComputeBuffer);
  convertResourceReference(manager, refDescriptorLayout);
  convertResourceReference(manager, refShader);
  convertResourceReference(manager, refPipeline);
  convertResourceReference(manager, refDescriptorSet);

  clearManagerReferences(manager);

  int ticks = 0;

  // Main loop
  while (windowIsOpen(window, engine)) {
    VIVIUM_LOG(LogSeverity::DEBUG, "Beginning frame..");

    std::vector<float> superCoolData = std::vector<float>(1024, 1.0f);

    engineBeginFrame(engine, ctx);
    Input::update(window);

    void* stagingMap = getBufferMapping(refComputeBuffer.resource);
    std::memcpy(stagingMap, superCoolData.data(),
                superCoolData.size() * sizeof(float));

    VIVIUM_LOG(LogSeverity::DEBUG, "Mapped data..");

    windowBeginFrame(window, ctx, engine);

    VIVIUM_LOG(LogSeverity::DEBUG, "Begun frame..");

    cmdBindPipeline(ctx, refPipeline.resource, PipelineBindPoint::COMPUTE);
    cmdBindDescriptorSet(ctx, refDescriptorSet.resource, refPipeline.resource,
                         PipelineBindPoint::COMPUTE);
    cmdDispatch(ctx, 16, 16, 1);

    windowEndFrame(window, engine);

    std::vector<float> outputData = std::vector<float>(1024, 0.0f);
    std::memcpy(outputData.data(), stagingMap,
                outputData.size() * sizeof(float));

    VIVIUM_LOG(LogSeverity::DEBUG, "Output data index 0: {}", outputData[0]);

    engineEndFrame(engine);

    ++ticks;
    if (ticks >= 3) {
      windowClose(window);
    }
  }

  // Cleanup
  dropBuffer(refComputeBuffer.resource, engine);
  dropDescriptorLayout(refDescriptorLayout.resource, engine);
  dropShader(refShader.resource, engine);
  dropPipeline(refPipeline.resource, engine);

  dropCommandContext(ctx, engine);
  dropManager(manager, engine);
  dropWindow(window, engine);
  dropEngine(engine);

  _fontTerminate();
}
}  // namespace Testing
