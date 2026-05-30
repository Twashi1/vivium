#pragma once

#include "buffer.h"
#include "descriptor_layout.h"
#include "shader.h"
#include "uniform.h"

namespace Vivium {
enum class _RenderTarget { FRAMEBUFFER, WINDOW, NONE };
enum class PipelineBindPoint {
  GRAPHICS = VK_PIPELINE_BIND_POINT_GRAPHICS,
  COMPUTE = VK_PIPELINE_BIND_POINT_COMPUTE
};

struct PipelineSpecification {
  std::vector<ShaderReference> shaders;
  BufferLayout bufferLayout;
  std::vector<DescriptorLayoutReference> descriptorLayouts;
  std::vector<PushConstant> pushConstants;
  PipelineBindPoint
      bindPoint;  // TODO: maybe should keep this metadata on the object itself?

  _RenderTarget target;

  union {
    VkRenderPass windowPass;
    FramebufferReference framebuffer;
  };

  VkSampleCountFlagBits sampleCount;

  PipelineSpecification() = default;

  static PipelineSpecification fromWindow(
      const std::span<const ShaderReference> shaders,
      const BufferLayout& bufferLayout,
      const std::span<const DescriptorLayoutReference> descriptorLayouts,
      const std::span<const PushConstant> pushConstants, Window& window);

  static PipelineSpecification fromFramebuffer(
      const std::span<const ShaderReference> shaders,
      const BufferLayout& bufferLayout,
      const std::span<const DescriptorLayoutReference> descriptorLayouts,
      const std::span<const PushConstant> pushConstants,
      FramebufferReference framebuffer, int multisampleCount);

  static PipelineSpecification fromCompute(
      const std::span<const ShaderReference> shaders,
      const BufferLayout& bufferLayout,
      const std::span<const DescriptorLayoutReference> descriptorLayouts,
      const std::span<const PushConstant> pushConstants);
};

struct Pipeline {
  VkPipelineLayout layout;
  VkPipeline pipeline;
  VkRenderPass renderPass;
};

struct PipelineReference {
  uint64_t referenceIndex;
};

void dropPipeline(Pipeline& pipeline, Engine& engine);
}  // namespace Vivium
