#pragma once

#include "../../engine.h"
#include "uniform.h"

namespace Vivium {
	// TODO: worrying data lifetimes
	struct DescriptorLayoutSpecification {
		std::vector<Uniform::Binding> bindings;
	};

	struct DescriptorLayout {
		VkDescriptorSetLayout layout;
	};

	struct DescriptorLayoutReference {
		uint64_t referenceIndex;
	};
		
	bool isDescriptorLayoutNull(DescriptorLayout const& layout);

	template <Storage::StorageType StorageType>
	void dropDescriptorLayout(StorageType* allocator, DescriptorLayout const& layout, Engine::Handle engine)
	{
		VIVIUM_NULLPTR_CHECK(engine, VIVIUM_CHECK_RESOURCE_EXISTS(*engine, Engine::isNull));

		vkDestroyDescriptorSetLayout(engine->device, layout.layout, nullptr);

		Storage::dropResource(allocator, &layout);
	}

	// TODO: needs to be on resource manager now
	/*
	template <Storage::StorageType StorageType>
	Handle create(StorageType* allocator, Engine::Handle engine, Specification specification)
	{
		Handle handle = Storage::allocateResource<Resource>(allocator);

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
		createInfo.bindingCount = static_cast<uint32_t>(vulkanBindings.size());
		createInfo.pBindings = vulkanBindings.data();

		vkCreateDescriptorSetLayout(engine->device, &createInfo, nullptr, &handle->layout);

		return handle;
	}
	*/
}