#include "descriptor_layout.h"

namespace Vivium {
	bool isDescriptorLayoutNull(DescriptorLayout const& layout)
	{
		return layout.layout == VK_NULL_HANDLE;
	}
}