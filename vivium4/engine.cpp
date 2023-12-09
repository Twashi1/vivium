#include "engine.h"

namespace Vivium {
	namespace Engine {
		VKAPI_ATTR VkBool32 VKAPI_CALL Resource::m_vulkanDebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT vkSeverity, 
			VkDebugUtilsMessageTypeFlagsEXT messageType, 
			const VkDebugUtilsMessengerCallbackDataEXT* callbackData, 
			void* userData
		)
		{
			Log::Severity severity;

			switch (vkSeverity) {
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:	severity = Log::DEBUG; break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:		severity = Log::DEBUG; break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:	severity = Log::WARN; break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:		severity = Log::ERROR; break;
			default:												severity = Log::INVALID; break;
			}

			VIVIUM_LOG(severity, "[VULKAN] {}", callbackData->pMessage);

			return VK_FALSE;
		}

		Resource::QueueFamilyIndices Resource::m_findQueueFamilies(VkPhysicalDevice device, Window::Handle window)
		{
			QueueFamilyIndices indices{ UINT32_MAX, UINT32_MAX, UINT32_MAX };

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

			for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
				const VkQueueFamilyProperties& properties = queueFamilyProperties[i];

				if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
					indices.graphicsFamily = i;
				if (properties.queueFlags & VK_QUEUE_TRANSFER_BIT)
					indices.transferFamily = i;
				
				// Checking present
				VkBool32 hasPresentSupport = false;
				VIVIUM_CHECK_RESOURCE(window);
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, window->m_surface, &hasPresentSupport);

				if (hasPresentSupport)
					indices.presentFamily = i;

				if (indices.isComplete())
					break;
			}

			return indices;
		}
		
		bool Resource::m_checkValidationLayerSupport(const std::vector<const char*>& validationLayers)
		{
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);

			// Iterate validation layers requested
			for (const char* layer : validationLayers) {
				bool layerFound = false;

				// Ensure each validation layer is found in properties
				for (const VkLayerProperties& properties : availableLayers) {
					if (strcmp(properties.layerName, layer) == 0) {
						layerFound = true;
						
						break;
					}
				}

				if (!layerFound) return false;
			}

			return true;
		}

		bool Resource::m_isSuitableDevice(VkPhysicalDevice device, const std::vector<const char*>& requiredExtensions, Window::Handle window)
		{
			QueueFamilyIndices queueFamilyIndices = m_findQueueFamilies(device, window);

			bool extensionSupport = m_checkDeviceExtensionSupport(requiredExtensions, device);
			bool swapChainAdequate = false;

			VkPhysicalDeviceFeatures supportedFeatures;
			vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

			if (extensionSupport) {
				
			}

			return false;
		}
		
		bool Resource::m_checkDeviceExtensionSupport(const std::vector<const char*>& requiredExtensions, VkPhysicalDevice device)
		{
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			for (const char* requiredExtension : requiredExtensions) {
				bool foundExtension = false;

				for (VkExtensionProperties availableExtension : availableExtensions) {
					// NOTE: extensionName is null-terminated
					if (strcmp(availableExtension.extensionName, requiredExtension) == 0)
					{
						foundExtension = true;

						break;
					}
				}

				if (!foundExtension) return false;
			}

			return true;
		}
	}
}