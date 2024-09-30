#include "pipeline.h"
#include "../resource_manager.h"

namespace Vivium {
	void dropPipeline(Pipeline& pipeline, Engine& engine)
	{
		vkDestroyPipelineLayout(engine.device, pipeline.layout, nullptr);
		vkDestroyPipeline(engine.device, pipeline.pipeline, nullptr);
	}

	PipelineSpecification PipelineSpecification::fromWindow(const std::span<const ShaderReference> shaders, const BufferLayout& bufferLayout, const std::span<const DescriptorLayoutReference> descriptorLayouts, const std::span<const PushConstant> pushConstants, Window& window)
	{
		PipelineSpecification specification;

		specification.shaders = std::vector<ShaderReference>(shaders.begin(), shaders.end());
		specification.bufferLayout = bufferLayout;
		specification.descriptorLayouts = std::vector<DescriptorLayoutReference>(descriptorLayouts.begin(), descriptorLayouts.end());
		specification.pushConstants = std::vector<PushConstant>(pushConstants.begin(), pushConstants.end());
		specification.windowPass = window.renderPass;
		specification.target = _RenderTarget::WINDOW;
		specification.sampleCount = window.multisampleCount;

		return specification;
	}
		
	PipelineSpecification PipelineSpecification::fromFramebuffer(const std::span<const ShaderReference> shaders, const BufferLayout& bufferLayout, const std::span<const DescriptorLayoutReference> descriptorLayouts, const std::span<const PushConstant> pushConstants, FramebufferReference framebuffer, int multisampleCount)
	{
		PipelineSpecification specification;

		specification.shaders = std::vector<ShaderReference>(shaders.begin(), shaders.end());
		specification.bufferLayout = bufferLayout;
		specification.descriptorLayouts = std::vector<DescriptorLayoutReference>(descriptorLayouts.begin(), descriptorLayouts.end());
		specification.pushConstants = std::vector<PushConstant>(pushConstants.begin(), pushConstants.end());
		specification.framebuffer = framebuffer;
		specification.target = _RenderTarget::FRAMEBUFFER;
		specification.sampleCount = static_cast<VkSampleCountFlagBits>(multisampleCount);

		return specification;
	}
}