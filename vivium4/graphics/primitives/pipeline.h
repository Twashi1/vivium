#pragma once

#include "shader.h"
#include "buffer.h"
#include "descriptor_layout.h"
#include "uniform.h"

namespace Vivium {
	namespace Pipeline {
		struct Specification {
			const std::span<const Shader::Handle> shaders;
			const Buffer::Layout bufferLayout;
			const std::span<const DescriptorLayout::Handle> descriptorLayouts;
			const std::span<const Uniform::PushConstant> pushConstants;

			Specification() = default;
			Specification(
				const std::span<const Shader::Handle> shaders,
				const Buffer::Layout bufferLayout,
				const std::span<const DescriptorLayout::Handle> descriptorLayouts,
				const std::span<const Uniform::PushConstant> pushConstants
			);
		};

		struct Resource {
			VkPipelineLayout layout;
			VkPipeline pipeline;

			bool isNull() const;
			void drop(Engine::Handle engine);
			void create(Engine::Handle engine, Window::Handle window, Specification specification);
		};	

		typedef Resource* Handle;

		// TODO: create method inside pipeline or constructor
		template <Allocator::AllocatorType AllocatorType>
		Handle create(AllocatorType allocator, Engine::Handle engine, Window::Handle window, Specification specification) {
			Handle handle = Allocator::allocateResource<Resource>(allocator);

			handle->create(engine, window, specification);

			return handle;
		}

		template <Allocator::AllocatorType AllocatorType>
		void drop(AllocatorType allocator, Handle handle, Engine::Handle engine)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine);
			VIVIUM_CHECK_HANDLE_EXISTS(handle);

			handle->drop(engine);

			Allocator::dropResource(allocator, handle);
		}
	}
}