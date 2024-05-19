#pragma once

#include "shader.h"
#include "buffer.h"
#include "descriptor_layout.h"
#include "uniform.h"

namespace Vivium {
	namespace Pipeline {
		struct Specification {
			std::vector<Shader::Handle> shaders;
			Buffer::Layout bufferLayout;
			std::vector<DescriptorLayout::Handle> descriptorLayouts;
			std::vector<Uniform::PushConstant> pushConstants;
			VkRenderPass renderPass;
			VkSampleCountFlagBits sampleCount;

			Specification() = default;
			// TODO: can't take in raw renderpass like this, user can't access it
			Specification(
				const std::span<const Shader::Handle> shaders,
				const Buffer::Layout bufferLayout,
				const std::span<const DescriptorLayout::Handle> descriptorLayouts,
				const std::span<const Uniform::PushConstant> pushConstants,
				VkRenderPass renderPass,
				VkSampleCountFlagBits sampleCount
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
		void drop(AllocatorType allocator, Handle handle, Engine::Handle engine)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);
			VIVIUM_CHECK_HANDLE_EXISTS(handle);

			vkDestroyPipelineLayout(engine->device, handle->layout, nullptr);
			vkDestroyPipeline(engine->device, handle->pipeline, nullptr);

			Allocator::dropResource(allocator, handle);
		}
	}
}