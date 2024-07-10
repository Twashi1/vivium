#pragma once

#include "../../engine.h"
#include "uniform.h"

namespace Vivium {
	struct DescriptorLayoutSpecification {
		std::vector<UniformBinding> bindings;
	};

	struct DescriptorLayout {
		VkDescriptorSetLayout layout;
	};

	struct DescriptorLayoutReference {
		uint64_t referenceIndex;
	};
		
	void dropDescriptorLayout(DescriptorLayout& layout, Engine::Handle engine);
}