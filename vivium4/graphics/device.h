#pragma once

#include "../core.h"

namespace Vivium {
	namespace Graphics {
		struct Device {
			VkDevice m_device;
			VkPhysicalDevice m_physicalDevice;
			VkInstance m_instance;

			VkQueue m_transferQueue;
			VkQueue m_graphicsQueue;
			VkQueue m_presentQueue;
		};
	}
}