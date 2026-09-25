#include "grid.h"

#include "state.h"

using namespace Vivium;

namespace TDGame {
void submitLevel(Level& level, State& state) {
  // Ref<Texture> texture;
  // Ref<Buffer> gridStorageBuffer;
  // Ref<Shader> gridVertexShader;
  // Ref<Shader> gridFragmentShader;
  // Ref<DescriptorSet> gridDescriptorSet;
  // Ref<DescriptorLayout> gridDescriptorLayout;
  // Ref<Pipeline> gridPipeline;

  submitResource(
      state.manager, &level.texture.reference,
      std::vector<TextureSpecification>({TextureSpecification::fromImageFile(
          "tdgame/res/images/atlas.png", TextureFormat::RGBA,
          TextureFilter::NEAREST)}));
  submitResource(
      state.manager, &level.gridStorageBuffer.reference, MemoryType::UNIFORM,
      std::vector<BufferSpecification>({BufferSpecification(
          32 * 32 * sizeof(_GridInstanceData), BufferUsage::STORAGE)}));

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
              {UniformData::fromBuffer(level.gridStorageBuffer.reference,
                                       32 * 32 * sizeof(_GridInstanceData), 0),
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
  std::vector<_GridInstanceData> data(32 * 32);

  for (uint32_t i = 0; i < 32 * 32; i++) {
    _GridInstanceData instance;

    AtlasIndex index = textureAtlasIndex(I32x2(256), I32x2(32),
                                         static_cast<int>(GridType::GRASS));
    uint32_t x = i % 32;
    uint32_t y = i / 32;
    F32x2 tileSize = F32x2(32.0f);
    F32x2 tilePosition = F32x2(x * tileSize.x, y * tileSize.y);

    instance.position = tilePosition;
    instance.size = tileSize;
    instance.texturePosition = index.translation;
    instance.textureSize = index.scale;
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
  cmdDrawIndexed(state.context, 6, 32 * 32);
}

void dropLevel(Level& level, State& state) {
  dropTexture(level.texture.resource, state.engine);
  dropBuffer(level.gridStorageBuffer.resource, state.engine);
  dropDescriptorLayout(level.gridDescriptorLayout.resource, state.engine);
  dropPipeline(level.gridPipeline.resource, state.engine);
}
}  // namespace TDGame
