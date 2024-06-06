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
		
	bool isDescriptorLayoutNull(DescriptorLayout const& layout);

	template <Storage::StorageType StorageType>
	void dropDescriptorLayout(StorageType* allocator, DescriptorLayout const& layout, Engine::Handle engine)
	{
		VIVIUM_NULLPTR_CHECK(engine, VIVIUM_CHECK_RESOURCE_EXISTS(*engine, Engine::isNull));

		vkDestroyDescriptorSetLayout(engine->device, layout.layout, nullptr);

		Storage::dropResource(allocator, &layout);
	}
}