#pragma once

#include "../../engine.h"
#include "uniform.h"

namespace Vivium {
	namespace DescriptorLayout {
		struct Resource {
			VkDescriptorSetLayout layout;
			std::vector<Uniform::Binding> bindings;

			bool isNull() const;
			void drop(Engine::Handle engine);
		};
		
		typedef Resource* Handle;

		struct Specification {
			const std::span<const Uniform::Binding> bindings;

			Specification();
			Specification(const std::span<const Uniform::Binding> bindings);
		};

		template <Allocator::AllocatorType AllocatorType>
		Handle create(AllocatorType allocator, Engine::Handle engine, Specification specification)
		{
			Handle handle = Allocator::allocateResource<Resource>(allocator);

			std::vector<VkDescriptorSetLayoutBinding> vulkanBindings(specification.bindings.size());
			handle->bindings.resize(specification.bindings.size());

			for (uint64_t i = 0; i < specification.bindings.size(); i++) {
				VkDescriptorSetLayoutBinding& vulkanBinding = vulkanBindings[i];
				const Uniform::Binding& binding = specification.bindings[i];

				handle->bindings[i] = binding;

				vulkanBinding.binding = binding.slot;
				vulkanBinding.descriptorCount = 1; // TODO: UBO arrays would require this to be editable
				vulkanBinding.descriptorType = static_cast<VkDescriptorType>(binding.type);
				vulkanBinding.stageFlags = static_cast<VkShaderStageFlags>(binding.stage);
				vulkanBinding.pImmutableSamplers = nullptr;
			}

			VkDescriptorSetLayoutCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			createInfo.bindingCount = vulkanBindings.size();
			createInfo.pBindings = vulkanBindings.data();

			vkCreateDescriptorSetLayout(engine->device, &createInfo, nullptr, &handle->layout);

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