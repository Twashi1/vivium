#pragma once

#include "shader.h"
#include "buffer.h"
#include "descriptor_layout.h"
#include "uniform.h"

namespace Vivium {
	namespace Pipeline {
		enum Target {
			FRAMEBUFFER,
			WINDOW
		};

		struct Specification {
			std::vector<Shader::Handle> shaders;
			Buffer::Layout bufferLayout;
			std::vector<DescriptorLayout::Handle> descriptorLayouts;
			std::vector<Uniform::PushConstant> pushConstants;
			
			// Render pass source
			Target target;

			union {
				Engine::Handle engine;
				Framebuffer::Handle framebuffer;
			};

			VkSampleCountFlagBits sampleCount;

			Specification() = default;

			static Specification fromWindow(
				const std::span<const Shader::Handle> shaders,
				const Buffer::Layout bufferLayout,
				const std::span<const DescriptorLayout::Handle> descriptorLayouts,
				const std::span<const Uniform::PushConstant> pushConstants,
				Engine::Handle engine,
				Window::Handle window
			);

			static Specification fromFramebuffer(
				const std::span<const Shader::Handle> shaders,
				const Buffer::Layout bufferLayout,
				const std::span<const DescriptorLayout::Handle> descriptorLayouts,
				const std::span<const Uniform::PushConstant> pushConstants,
				Framebuffer::Handle framebuffer,
				int multisampleCount
			);
		};

		struct Resource {
			VkPipelineLayout layout;
			VkPipeline pipeline;
			VkRenderPass renderPass;
		};	

		typedef Resource* Handle;
		typedef Resource* PromisedHandle;

		bool isNull(const Handle pipeline);

		template <Allocator::AllocatorType AllocatorType>
		void drop(AllocatorType* allocator, Handle handle, Engine::Handle engine)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);
			VIVIUM_CHECK_HANDLE_EXISTS(handle);

			vkDestroyPipelineLayout(engine->device, handle->layout, nullptr);
			vkDestroyPipeline(engine->device, handle->pipeline, nullptr);

			Allocator::dropResource(allocator, handle);
		}
	}
}