#pragma once

#include "descriptor_layout.h"

namespace Vivium {
	namespace DescriptorSet {
		struct Resource {
			VkDescriptorSet descriptorSet;

			bool isNull() const;
			// No need for drop
		};

		struct Specification {
			DescriptorLayout::Handle layout;
			std::vector<Uniform::Data> uniforms;

			Specification() = default;
			Specification(DescriptorLayout::Handle layout, const std::span<const Uniform::Data> uniforms);
		};

		typedef Resource* Handle;
	}
}