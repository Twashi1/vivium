#include "pipeline.h"
#include "../resource_manager.h"

namespace Vivium {
	PipelineSpecification PipelineSpecification::fromWindow(const std::span<const ShaderReference> shaders, const BufferLayout& bufferLayout, const std::span<const DescriptorLayoutReference> descriptorLayouts, const std::span<const Uniform::PushConstant> pushConstants, Engine::Handle engine, Window::Handle window)
	{
		PipelineSpecification specification;

		specification.shaders = std::vector<ShaderReference>(shaders.begin(), shaders.end());
		specification.bufferLayout = bufferLayout;
		specification.descriptorLayouts = std::vector<DescriptorLayoutReference>(descriptorLayouts.begin(), descriptorLayouts.end());
		specification.pushConstants = std::vector<Uniform::PushConstant>(pushConstants.begin(), pushConstants.end());
		specification.engine = engine;
		specification.target = _RenderTarget::WINDOW;
		specification.sampleCount = window->multisampleCount;

		return specification;
	}
		
	PipelineSpecification PipelineSpecification::fromFramebuffer(const std::span<const ShaderReference> shaders, const BufferLayout& bufferLayout, const std::span<const DescriptorLayoutReference> descriptorLayouts, const std::span<const Uniform::PushConstant> pushConstants, FramebufferReference framebuffer, int multisampleCount)
	{
		PipelineSpecification specification;

		specification.shaders = std::vector<ShaderReference>(shaders.begin(), shaders.end());
		specification.bufferLayout = bufferLayout;
		specification.descriptorLayouts = std::vector<DescriptorLayoutReference>(descriptorLayouts.begin(), descriptorLayouts.end());
		specification.pushConstants = std::vector<Uniform::PushConstant>(pushConstants.begin(), pushConstants.end());
		specification.framebuffer = framebuffer;
		specification.target = _RenderTarget::FRAMEBUFFER;
		specification.sampleCount = static_cast<VkSampleCountFlagBits>(multisampleCount);

		return specification;
	}
}