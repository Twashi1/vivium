#include "memory_type.h"

namespace Vivium {
	bool isMappable(MemoryType type) {
		return static_cast<VkMemoryPropertyFlags>(type) & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	}
	
	uint32_t findMemoryType(Engine& engine, uint32_t typeFilter, VkMemoryPropertyFlags memoryProperties)
	{
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
		vkGetPhysicalDeviceMemoryProperties(
			engine.physicalDevice,
			&deviceMemoryProperties
		);

		for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) &&
				(deviceMemoryProperties.memoryTypes[i].propertyFlags & memoryProperties) == memoryProperties) {
				return i;
			}
		}

		VIVIUM_LOG(LogSeverity::FATAL, "Failed to find suitable memory type");

		return NULL;
	}
}