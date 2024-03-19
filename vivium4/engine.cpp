#include "engine.h"

namespace Vivium {
	namespace Engine {
		void Resource::populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT createInfo)
		{
			createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createInfo.pfnUserCallback = vulkanDebugCallback;
		}

		VKAPI_ATTR VkBool32 VKAPI_CALL Resource::vulkanDebugCallback(
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

		Resource::QueueFamilyIndices Resource::findQueueFamilies(VkPhysicalDevice device, Window::Handle window)
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
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, window->surface, &hasPresentSupport);

				if (hasPresentSupport)
					indices.presentFamily = i;

				if (indices.isComplete())
					break;
			}

			return indices;
		}
		
		bool Resource::checkValidationLayerSupport(const std::span<const char* const>& validationLayers)
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

		bool Resource::isSuitableDevice(VkPhysicalDevice device, const std::vector<const char*>& requiredExtensions, Window::Handle window)
		{
			QueueFamilyIndices queueFamilyIndices = findQueueFamilies(device, window);

			bool extensionSupport = checkDeviceExtensionSupport(requiredExtensions, device);
			bool swapChainAdequate = false;

			VkPhysicalDeviceFeatures supportedFeatures;
			vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

			if (extensionSupport) {
				
			}

			return false;
		}
		
		bool Resource::checkDeviceExtensionSupport(const std::vector<const char*>& requiredExtensions, VkPhysicalDevice device)
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

		Resource::SwapChainSupportDetails Resource::querySwapChainSupport(VkPhysicalDevice device, Window::Handle window)
		{
			SwapChainSupportDetails details;

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, window->surface, &details.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, window->surface, &formatCount, nullptr);

			if (formatCount != 0) {
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, window->surface, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, window->surface, &presentModeCount, nullptr);

			if (presentModeCount != 0) {
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, window->surface, &presentModeCount, details.presentModes.data());
			}

			return details;
		}

		void Resource::pickPhysicalDevice(const std::vector<const char*>& extensions, Window::Handle window)
		{
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

			if (deviceCount == 0) {
				VIVIUM_LOG(Log::FATAL, "Couldn't find GPU with vulkan support");
			}

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

			for (const VkPhysicalDevice& device : devices) {
				if (isSuitableDevice(device, extensions, window)) {
					physicalDevice = device;

					break;
				}
			}

			VIVIUM_ASSERT(physicalDevice != VK_NULL_HANDLE, "Failed to find suitable GPU");
		}

		std::vector<const char*> Resource::createInstance(const std::span<const char* const> validationLayers, const std::span<const char* const> defaultExtensions)
		{
			if (enableValidationLayers && !checkValidationLayerSupport(validationLayers)) {
				VIVIUM_LOG(Log::FATAL, "Requested validation layers, but some not available");
			}

			VkApplicationInfo appInfo{};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "Vulkan App";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "No Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_0;

			VkInstanceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;

			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions;

			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

			if (enableValidationLayers) {
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}

			createInfo.enabledExtensionCount = extensions.size();
			createInfo.ppEnabledExtensionNames = extensions.data();

			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

			if (enableValidationLayers) {
				createInfo.enabledLayerCount = validationLayers.size();
				createInfo.ppEnabledLayerNames = validationLayers.data();

				populateDebugMessengerInfo(debugCreateInfo);
				createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
			}
			else {
				createInfo.enabledLayerCount = 0;
				createInfo.pNext = nullptr;
			}

			VIVIUM_VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance),
				"Failed to create vulkan instance");

			return extensions;
		}

		void Resource::setupDebugMessenger()
		{
			if (!enableValidationLayers) return;

			VkDebugUtilsMessengerCreateInfoEXT createInfo{};
			populateDebugMessengerInfo(createInfo);

			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			VkResult result;

			if (func != nullptr) {
				result = func(instance, &createInfo, nullptr, &debugMessenger);
			}
			else {
				result = VK_ERROR_EXTENSION_NOT_PRESENT;
			}

			VIVIUM_VK_CHECK(result, "Failed to create debug messenger");
		}
		
		void Resource::create(Options options, Window::Handle window)
		{
			const std::array<const char* const, 2> defaultExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME,
				VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
			};

			const std::array<const char* const, 1> validationLayers = {
				"VK_LAYER_KHRONOS_validation"
			};

			std::vector<const char*> extensions = createInstance(validationLayers, defaultExtensions);
			setupDebugMessenger();

			window->createSurface(this);

			pickPhysicalDevice(extensions, window);

			// TODO: set options

			createLogicalDevice();

			/*

			-m_create_swap_chain();
			-m_create_image_views();
			-m_create_multisample_color_images();

			*/

			window->initVulkan(this);

			createRenderPass();

			createFramebuffers();

			createCommandPool();
			createCommandBuffers();

			createSyncObjects();
		}
	}
}