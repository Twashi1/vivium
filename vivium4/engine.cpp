#include "engine.h"
#include "graphics/commands.h"

namespace Vivium {
	void _populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = _vulkanDebugCallback;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL _vulkanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT vkSeverity, 
		VkDebugUtilsMessageTypeFlagsEXT messageType, 
		const VkDebugUtilsMessengerCallbackDataEXT* callbackData, 
		void* userData
	)
	{
		LogSeverity severity;

		switch (vkSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:	severity = LogSeverity::DEBUG; break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:		severity = LogSeverity::DEBUG; break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:	severity = LogSeverity::WARN; break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:		severity = LogSeverity::ERROR; break;
		default: return VK_FALSE;
		}

		VIVIUM_LOG(severity, "[VULKAN LOG] {}", callbackData->pMessage);

		return VK_FALSE;
	}

	Engine::QueueFamilyIndices _findQueueFamilies(VkPhysicalDevice device)
	{
		// TODO: look for dedicated transfer queue?

		Engine::QueueFamilyIndices indices{ UINT32_MAX, UINT32_MAX, UINT32_MAX };

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

		for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
			const VkQueueFamilyProperties& properties = queueFamilyProperties[i];

			if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
				// We can pretty safely make the assumption that any graphics queue also supports present
				indices.presentFamily = i;
			}
			if (properties.queueFlags & VK_QUEUE_TRANSFER_BIT)
				indices.transferFamily = i;

			if (indices.graphicsFamily != UINT32_MAX && indices.presentFamily != UINT32_MAX && indices.transferFamily != UINT32_MAX)
				break;
		}

		VIVIUM_ASSERT(indices.graphicsFamily != UINT32_MAX && indices.presentFamily != UINT32_MAX && indices.transferFamily != UINT32_MAX, "Couldn't complete queue indices");

		return indices;
	}
		
	bool _checkValidationLayerSupport(const std::span<const char* const>& validationLayers)
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

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

	bool _checkSurfaceSupport(Engine& engine, VkSurfaceKHR surface)
	{
		Engine::QueueFamilyIndices queueFamilyIndices = _findQueueFamilies(engine.physicalDevice);

		// Validate present support
		VkBool32 isPresentSupported;
		VIVIUM_VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(engine.physicalDevice, queueFamilyIndices.presentFamily, surface, &isPresentSupported),
			"Failed to get physical device surface support details");
		VIVIUM_ASSERT(isPresentSupported, "Present not supported for selected physical device");

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(engine.physicalDevice, &supportedFeatures);

		Engine::SwapChainSupportDetails swapChainSupport = _querySwapChainSupport(engine.physicalDevice, surface);
		bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

		return swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}
		
	bool _checkDeviceExtensionSupport(const std::vector<const char*>& requiredExtensions, VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		for (const char* requiredExtension : requiredExtensions) {
			bool foundExtension = false;

			for (VkExtensionProperties& availableExtension : availableExtensions) {
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

	Engine::SwapChainSupportDetails _querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		Engine::SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	void _setOptions(Engine& engine, EngineOptions const& options)
	{
		VIVIUM_ASSERT(options.fps > 0, "Can't have 0 FPS");

		engine.targetTimePerFrame = 1.0f / options.fps;
		engine.pollPeriod = options.pollPeriod;
	}

	void _pickPhysicalDevice(Engine& engine, const std::vector<const char*>& deviceExtensions)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(engine.instance, &deviceCount, nullptr);

		VIVIUM_ASSERT(deviceCount > 0, "Failed to find GPU with Vulkan support");
		VIVIUM_LOG(LogSeverity::DEBUG, "Found {} devices", deviceCount);

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(engine.instance, &deviceCount, devices.data());

		// Select first device that has discrete GPU, then integrated GPU
		bool isIntegrated = false;

		for (uint64_t deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++) {
			VkPhysicalDevice device = devices[deviceIndex];

			// Validate extension support for device
			if (!_checkDeviceExtensionSupport(deviceExtensions, device)) continue;

			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(device, &properties);

			switch (properties.deviceType) {
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				engine.physicalDevice = device; return;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				engine.physicalDevice = device; isIntegrated = true; break;
			default:
				if (!isIntegrated) engine.physicalDevice = device; break;
			}
		}

		VIVIUM_ASSERT(engine.physicalDevice != VK_NULL_HANDLE, "Failed to find suitable GPU");
	}

	void _createLogicalDevice(Engine& engine, const std::span<const char* const> extensions, const std::span<const char* const> validationLayers)
	{
		Engine::QueueFamilyIndices indices = _findQueueFamilies(engine.physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {
			indices.graphicsFamily,
			indices.presentFamily,
			indices.transferFamily
		};

		float queuePriority = 1.0f;

		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (Engine::enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		VIVIUM_VK_CHECK(vkCreateDevice(engine.physicalDevice, &createInfo, nullptr, &engine.device),
			"Failed to create logical device");

		vkGetDeviceQueue(engine.device, indices.graphicsFamily, 0, &engine.graphicsQueue);
		vkGetDeviceQueue(engine.device, indices.presentFamily, 0, &engine.presentQueue);
		vkGetDeviceQueue(engine.device, indices.transferFamily, 0, &engine.transferQueue);
	}

	void _createInstance(Engine& engine, const std::span<const char* const> validationLayers, const std::span<const char* const> defaultExtensions)
	{
		if (Engine::enableValidationLayers && !_checkValidationLayerSupport(validationLayers)) {
			VIVIUM_LOG(LogSeverity::FATAL, "Requested validation layers, but some not available");
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

		if (Engine::enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

		if (Engine::enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			_populateDebugMessengerInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		VIVIUM_VK_CHECK(vkCreateInstance(&createInfo, nullptr, &engine.instance),
			"Failed to create vulkan instance");
	}

	void _setupDebugMessenger(Engine& engine)
	{
		if (!Engine::enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		_populateDebugMessengerInfo(createInfo);

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(engine.instance, "vkCreateDebugUtilsMessengerEXT");
		VkResult returnValue;

		if (func != nullptr) {
			returnValue = func(engine.instance, &createInfo, nullptr, &engine.debugMessenger);
		}
		else {
			returnValue = VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		VIVIUM_VK_CHECK(returnValue, "Failed to create debug messenger");
	}

	void _checkPerformance(Engine& engine)
	{
		float maxTimeSpent = engine.targetTimePerFrame * engine.pollFramesCounted;

		if (maxTimeSpent > engine.pollPeriod) {
			float calculatedTimePerFrame = engine.pollFramesElapsedTime / engine.pollFramesCounted;
			float calculatedTimePerUpdate = engine.pollUpdatesElapsedTime / engine.pollFramesCounted;

			VIVIUM_LOG(LogSeverity::DEBUG,
				"Max FPS: {}, True FPS: {}, TPF: {}ms",
				1.0f / calculatedTimePerFrame,
				1.0f / calculatedTimePerUpdate,
				calculatedTimePerFrame * 1000.0f
			);

			float delta_tpf = calculatedTimePerFrame - engine.targetTimePerFrame;

			if (delta_tpf > 0.0f)
				VIVIUM_LOG(LogSeverity::WARN, "Running behind by {}ms on average",
					delta_tpf * 1000.0f);

			engine.pollFramesCounted = 0;
			engine.pollFramesElapsedTime = 0.0f;
			engine.pollUpdatesElapsedTime = 0.0f;
		}
	}

	void _limitFramerate(Engine& engine)
	{
		float elapsed = engine.frameTimer.getTime();

		engine.pollFramesElapsedTime += elapsed;
		++engine.pollFramesCounted;

		float sleep_time = engine.targetTimePerFrame - elapsed;

		if (sleep_time > 0.0f) {
			Time::nanosleep(static_cast<long long>(static_cast<double>(sleep_time) * 1.0E9));
		}

		engine.frameTimer.reset();
		engine.pollUpdatesElapsedTime += engine.updateTimer.reset();
	}

	Engine createEngine(EngineOptions const& options)
	{
		Engine engine;
		engine.instance = VK_NULL_HANDLE;
		engine.debugMessenger = VK_NULL_HANDLE;
		engine.physicalDevice = VK_NULL_HANDLE;
		engine.device = VK_NULL_HANDLE;
		engine.graphicsQueue = VK_NULL_HANDLE;
		engine.presentQueue = VK_NULL_HANDLE;
		engine.transferQueue = VK_NULL_HANDLE;
		engine.targetTimePerFrame = 0.0f;
		engine.pollPeriod = 0.0f;
		engine.pollFramesElapsedTime = 0.0f;
		engine.pollUpdatesElapsedTime = 0.0f;
		engine.pollFramesCounted = 0;

		_setOptions(engine, options);

		if (!glfwInit()) {
			VIVIUM_LOG(LogSeverity::FATAL, "GLFW failed to initialise");
		}

		// TODO: better error callback for glfw?
		glfwSetErrorCallback([](int code, const char* desc) {
			VIVIUM_LOG(LogSeverity::ERROR, "[GLFW {}] {}", code, desc);
			});

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		const std::array<const char* const, 2> defaultExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
		};

		std::vector<const char*> deviceExtensions;

		for (const char* extension : defaultExtensions) {
			deviceExtensions.push_back(extension);
		}

		const std::array<const char* const, 1> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		_createInstance(engine, validationLayers, defaultExtensions);
		_setupDebugMessenger(engine);

		_pickPhysicalDevice(engine, deviceExtensions);
		_createLogicalDevice(engine, deviceExtensions, validationLayers);

		return engine;
	}

	void dropEngine(Engine& engine) {
		vkDeviceWaitIdle(engine.device);

		vkDestroyDevice(engine.device, nullptr);

		if (Engine::enableValidationLayers) {
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(engine.instance, "vkDestroyDebugUtilsMessengerEXT");
				
			if (func != nullptr) {
				func(engine.instance, engine.debugMessenger, nullptr);
			}
		}

		vkDestroyInstance(engine.instance, nullptr);

		glfwTerminate();
	}
		
	void engineBeginFrame(Engine& engine, CommandContext& context)
	{
		glfwPollEvents();
		_contextFlush(context, engine);
		// TODO: update input and GUI?
	}
		
	void engineEndFrame(Engine& engine)
	{
		_checkPerformance(engine);
		_limitFramerate(engine);
	}
}