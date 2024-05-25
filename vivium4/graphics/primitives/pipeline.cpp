#include "pipeline.h"
#include "../resource_manager.h"

namespace Vivium {
	namespace Pipeline {
		bool isNull(const Handle pipeline)
		{
			return pipeline->pipeline == VK_NULL_HANDLE;
		}

		void drop(ResourceManager::Static::Handle manager, Pipeline::Handle pipeline, Engine::Handle engine)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);
			VIVIUM_CHECK_HANDLE_EXISTS(manager);
			VIVIUM_CHECK_HANDLE_EXISTS(pipeline);

			vkDestroyPipelineLayout(engine->device, pipeline->layout, nullptr);
			vkDestroyPipeline(engine->device, pipeline->pipeline, nullptr);
		}

		Specification Specification::fromWindow(const std::span<const Shader::Handle> shaders, const Buffer::Layout bufferLayout, const std::span<const DescriptorLayout::Handle> descriptorLayouts, const std::span<const Uniform::PushConstant> pushConstants, Engine::Handle engine, Window::Handle window)
		{
			Specification specification;

			specification.shaders = std::vector<Shader::Handle>(shaders.begin(), shaders.end());
			specification.bufferLayout = bufferLayout;
			specification.descriptorLayouts = std::vector<DescriptorLayout::Handle>(descriptorLayouts.begin(), descriptorLayouts.end());
			specification.pushConstants = std::vector<Uniform::PushConstant>(pushConstants.begin(), pushConstants.end());
			specification.engine = engine;
			specification.target = Target::WINDOW;
			specification.sampleCount = window->multisampleCount;

			return specification;
		}
		
		Specification Specification::fromFramebuffer(const std::span<const Shader::Handle> shaders, const Buffer::Layout bufferLayout, const std::span<const DescriptorLayout::Handle> descriptorLayouts, const std::span<const Uniform::PushConstant> pushConstants, Framebuffer::Handle framebuffer, int multisampleCount)
		{
			Specification specification;

			specification.shaders = std::vector<Shader::Handle>(shaders.begin(), shaders.end());
			specification.bufferLayout = bufferLayout;
			specification.descriptorLayouts = std::vector<DescriptorLayout::Handle>(descriptorLayouts.begin(), descriptorLayouts.end());
			specification.pushConstants = std::vector<Uniform::PushConstant>(pushConstants.begin(), pushConstants.end());
			specification.framebuffer = framebuffer;
			specification.target = Target::FRAMEBUFFER;
			specification.sampleCount = static_cast<VkSampleCountFlagBits>(multisampleCount);

			return specification;
		}
	}
}