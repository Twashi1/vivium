#pragma once

#include "../../engine.h"
#include "uniform.h"

namespace Vivium {
	// TODO: worrying data lifetimes
	struct DescriptorLayoutSpecification {
		std::vector<UniformBinding> bindings;
	};

	struct DescriptorLayout {
		VkDescriptorSetLayout layout;
	};

	struct DescriptorLayoutReference {
		uint64_t referenceIndex;
	};
		
	template <Storage::StorageType StorageType>
	void dropDescriptorLayout(StorageType* allocator, DescriptorLayout const& layout, Engine::Handle engine)
	{
		vkDestroyDescriptorSetLayout(engine->device, layout.layout, nullptr);

		Storage::dropResource(allocator, &layout);
	}
}