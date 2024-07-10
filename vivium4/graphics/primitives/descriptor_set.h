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
}