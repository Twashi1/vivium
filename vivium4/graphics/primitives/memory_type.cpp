#include "memory_type.h"

namespace Vivium {
	namespace Graphics {
		bool isMappable(MemoryType type) {
			return static_cast<VkMemoryPropertyFlags>(type) & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		}
	}
}