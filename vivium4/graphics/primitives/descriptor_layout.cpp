#include "descriptor_layout.h"

namespace Vivium {
	namespace DescriptorLayout {
		bool isNull(const Handle layout)
		{
			return layout->layout == VK_NULL_HANDLE;
		}
		
		Specification::Specification(const std::span<const Uniform::Binding> bindings)
			: bindings(bindings)
		{}
	}
}