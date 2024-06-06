#include "descriptor_set.h"

namespace Vivium {
	bool isDescriptorSetNull(DescriptorSet const& set)
	{
		return set.descriptorSet == VK_NULL_HANDLE;
	}
}