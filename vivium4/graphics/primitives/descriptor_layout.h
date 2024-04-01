#pragma once

#include "../../engine.h"
#include "uniform.h"

namespace Vivium {
	namespace DescriptorLayout {
		struct Specification {
			const std::span<const Uniform::Binding> bindings;

			Specification();
			Specification(const std::span<const Uniform::Binding> bindings);
		};

		struct Resource {
			VkDescriptorSetLayout layout;
			std::vector<Uniform::Binding> bindings;

			bool isNull() const;
			void drop(Engine::Handle engine);
			void create(Engine::Handle engine, Specification specification);
		};
		
		typedef Resource* Handle;

		template <Allocator::AllocatorType AllocatorType>
		Handle create(AllocatorType allocator, Engine::Handle engine, Specification specification)
		{
			Handle handle = Allocator::allocateResource<Resource>(allocator);

			handle->create(engine, specification);

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