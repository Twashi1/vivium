#include "descriptor_set.h"

namespace Vivium {
	namespace DescriptorSet {
		bool isNull(const Handle set)
		{
			return set->descriptorSet == VK_NULL_HANDLE;
		}

		Specification::Specification(DescriptorLayout::Handle layout, const std::span<const Uniform::Data> uniforms)
			: layout(layout)
		{
			this->uniforms.resize(uniforms.size());
			std::memcpy(this->uniforms.data(), uniforms.data(), uniforms.size_bytes());
		}
	}
}