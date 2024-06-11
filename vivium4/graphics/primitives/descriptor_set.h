#pragma once

#include "descriptor_layout.h"

namespace Vivium {
	struct DescriptorSet {
		VkDescriptorSet descriptorSet;
	};

	struct DescriptorSetReference {
		uint64_t referenceIndex;
	};

	struct DescriptorSetSpecification {
		DescriptorLayoutReference layout;
		std::vector<UniformData> uniforms;
	};

	bool isDescriptorSetNull(DescriptorSet const& set);

	template <Storage::StorageType StorageType>
	void dropDescriptorSet(StorageType* allocator, DescriptorSet& set) {
		// No need to do anything else, descriptor set has no destructor
		Storage::dropResource(allocator, &set);
	}
}