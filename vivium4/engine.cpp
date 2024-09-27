#include "engine.h"

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
		Log::Severity severity;

		switch (vkSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:	severity = Log::DEBUG; break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:		severity = Log::DEBUG; break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:	severity = Log::WARN; break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:		severity = Log::ERROR; break;
		default:												severity = Log::INVALID; break;
		}

		VIVIUM_LOG(severity, "[VULKAN LOG] {}", callbackData->pMessage);

		return VK_FALSE;
	}

	Engine::QueueFamilyIndices _findQueueFamilies(Engine& engine, VkPhysicalDevice device, Window& window)
	{
		Engine::QueueFamilyIndices indices{ UINT32_MAX, UINT32_MAX, UINT32_MAX };

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
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, window.surface, &hasPresentSupport);

			if (hasPresentSupport)
				indices.presentFamily = i;

			if (indices.isComplete())
				break;
		}

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

	bool _isSuitableDevice(Engine& engine, VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions, Window& window)
	{
		Engine::QueueFamilyIndices queueFamilyIndices = _findQueueFamilies(engine, device, window);

		// TODO: shouldn't be all extensions, just device ones
		bool extensionSupport = _checkDeviceExtensionSupport(deviceExtensions, device);
		bool swapChainAdequate = false;

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		if (extensionSupport) {
			Engine::SwapChainSupportDetails swapChainSupport = _querySwapChainSupport(device, window);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return queueFamilyIndices.isComplete() && extensionSupport && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}
		
	bool _checkDeviceExtensionSupport(const std::vector<const char*>& requiredExtensions, VkPhysicalDevice device)
	{
		// TODO: ?
		// NOTE: different

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

	Engine::SwapChainSupportDetails _querySwapChainSupport(VkPhysicalDevice device, Window& window)
	{
		Engine::SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, window.surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, window.surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, window.surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, window.surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, window.surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	void _setOptions(Engine& engine, EngineOptions const& options)
	{
		VIVIUM_ASSERT(options.fps > 0, "Can't have 0fps");

		engine.targetTimePerFrame = 1.0f / options.fps;
		engine.pollPeriod = options.pollPeriod;
	}

	void _pickPhysicalDevice(Engine& engine, const std::vector<const char*>& deviceExtensions, Window& window)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(engine.instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			VIVIUM_LOG(Log::FATAL, "Couldn't find GPU with vulkan support");
		}
			
		VIVIUM_LOG(Log::DEBUG, "Found {} devices", deviceCount);

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(engine.instance, &deviceCount, devices.data());

		// TODO: change back
		for (uint64_t deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++) {
			VkPhysicalDevice device = devices[deviceIndex];

			if (_isSuitableDevice(engine, device, deviceExtensions, window)) {
				engine.physicalDevice = device;

				VIVIUM_LOG(Log::DEBUG, "Selected device {}", deviceIndex);

				break;
			}
		}

		VIVIUM_ASSERT(engine.physicalDevice != VK_NULL_HANDLE, "Failed to find suitable GPU");
	}

	void _createLogicalDevice(Engine& engine, Window& window, const std::span<const char* const> extensions, const std::span<const char* const> validationLayers)
	{
		Engine::QueueFamilyIndices indices = _findQueueFamilies(engine, engine.physicalDevice, window);

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

	void _createEngineCommandPool(Engine& engine, Window& window)
	{
		Engine::QueueFamilyIndices queueFamilyIndices = _findQueueFamilies(engine, engine.physicalDevice, window);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

		VIVIUM_VK_CHECK(vkCreateCommandPool(engine.device, &poolInfo, nullptr, &engine.commandPool),
			"Failed to create command pool");
	}

	void _createEngineCommandBuffers(Engine& engine)
	{
		engine.commandBuffers.resize(Engine::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = engine.commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = static_cast<uint32_t>(engine.commandBuffers.size());

		VIVIUM_VK_CHECK(vkAllocateCommandBuffers(engine.device, &allocateInfo, engine.commandBuffers.data()),
			"Failed to allocate command buffers");
	}

	void _createSyncObjects(Engine& engine)
	{
		engine.imageAvailableSemaphores.resize(Engine::MAX_FRAMES_IN_FLIGHT);
		engine.renderFinishedSemaphores.resize(Engine::MAX_FRAMES_IN_FLIGHT);
		engine.inFlightFences.resize(Engine::MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint32_t i = 0; i < Engine::MAX_FRAMES_IN_FLIGHT; i++) {
			// TODO: VkCheck this
			if (vkCreateSemaphore(engine.device, &semaphoreInfo, nullptr, &engine.imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(engine.device, &semaphoreInfo, nullptr, &engine.renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(engine.device, &fenceInfo, nullptr, &engine.inFlightFences[i]) != VK_SUCCESS) {
				VIVIUM_LOG(Log::FATAL, "Failed to create sync objects for a frame");
			}
		}
	}

	void _createInstance(Engine& engine, const std::span<const char* const> validationLayers, const std::span<const char* const> defaultExtensions)
	{
		if (Engine::enableValidationLayers && !_checkValidationLayerSupport(validationLayers)) {
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

	uint32_t _findMemoryType(Engine& engine, uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(
			engine.physicalDevice,
			&memoryProperties
		);

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) &&
				(memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		VIVIUM_LOG(Log::FATAL, "Failed to find suitable memory type");

		return NULL;
	}

	void _checkPerformance(Engine& engine)
	{
		float maxTimeSpent = engine.targetTimePerFrame * engine.pollFramesCounted;

		if (maxTimeSpent > engine.pollPeriod) {
			float calculatedTimePerFrame = engine.pollFramesElapsedTime / engine.pollFramesCounted;
			float calculatedTimePerUpdate = engine.pollUpdatesElapsedTime / engine.pollFramesCounted;

			VIVIUM_LOG(Log::DEBUG,
				"Max FPS: {}, True FPS: {}, TPF: {}ms",
				1.0f / calculatedTimePerFrame,
				1.0f / calculatedTimePerUpdate,
				calculatedTimePerFrame * 1000.0f
			);

			float delta_tpf = calculatedTimePerFrame - engine.targetTimePerFrame;

			if (delta_tpf > 0.0f)
				VIVIUM_LOG(Log::WARN, "Running behind by {}ms on average",
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

	Engine createEngine(EngineOptions const& options, Window& window)
	{
		Engine engine;
		engine.instance = VK_NULL_HANDLE;
		engine.debugMessenger = VK_NULL_HANDLE;
		engine.physicalDevice = VK_NULL_HANDLE;
		engine.device = VK_NULL_HANDLE;
		engine.renderPass = VK_NULL_HANDLE;
		engine.commandPool = VK_NULL_HANDLE;
		engine.graphicsQueue = VK_NULL_HANDLE;
		engine.presentQueue = VK_NULL_HANDLE;
		engine.transferQueue = VK_NULL_HANDLE;
		engine.currentFrameIndex = 0;
		engine.currentImageIndex = UINT32_MAX;
		engine.targetTimePerFrame = 0.0f;
		engine.pollPeriod = 0.0f;
		engine.pollFramesElapsedTime = 0.0f;
		engine.pollUpdatesElapsedTime = 0.0f;
		engine.pollFramesCounted = 0;

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

		_createSurface(window, engine);

		_pickPhysicalDevice(engine, deviceExtensions, window);

		_setOptions(engine, options);

		_createLogicalDevice(engine, window, deviceExtensions, validationLayers);

		_initVulkan(window, engine);

		_createEngineCommandPool(engine, window);
		_createEngineCommandBuffers(engine);

		_createSyncObjects(engine);

		return engine;
	}

	void dropEngine(Engine& engine, Window& window) {
		vkDeviceWaitIdle(engine.device);

		vkDestroyRenderPass(engine.device, engine.renderPass, nullptr);

		for (uint32_t i = 0; i < Engine::MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(engine.device, engine.renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(engine.device, engine.imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(engine.device, engine.inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(engine.device, engine.commandPool, nullptr);

		vkDestroyDevice(engine.device, nullptr);

		if (Engine::enableValidationLayers) {
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(engine.instance, "vkDestroyDebugUtilsMessengerEXT");
				
			if (func != nullptr) {
				func(engine.instance, engine.debugMessenger, nullptr);
			}
		}

		vkDestroySurfaceKHR(engine.instance, window.surface, nullptr);
		vkDestroyInstance(engine.instance, nullptr);

		glfwTerminate();
	}
		
	void engineBeginFrame(Engine& engine, Window& window)
	{
		glfwPollEvents();
		vkWaitForFences(engine.device, 1, &engine.inFlightFences[engine.currentFrameIndex], VK_TRUE, UINT64_MAX);

		VkResult acquireImageResult = vkAcquireNextImageKHR(
			engine.device,
			window.swapChain,
			UINT64_MAX,
			engine.imageAvailableSemaphores[engine.currentFrameIndex],
			VK_NULL_HANDLE,
			&engine.currentImageIndex
		);

		if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
			_recreateSwapChain(window, engine);

			return;
		}
		else if (acquireImageResult != VK_SUCCESS && acquireImageResult != VK_SUBOPTIMAL_KHR) {
			VIVIUM_LOG(Log::FATAL, "Failed to acquire swapchain image");
		}

		vkResetFences(engine.device, 1, &engine.inFlightFences[engine.currentFrameIndex]);

		vkResetCommandBuffer(engine.commandBuffers[engine.currentFrameIndex], 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VIVIUM_VK_CHECK(vkBeginCommandBuffer(engine.commandBuffers[engine.currentFrameIndex], &beginInfo),
			"Failed to begin recording command buffer");
	}
		
	void engineEndFrame(Engine& engine, Window& window)
	{
		// TODO: fix variable naming style conventions

		VIVIUM_VK_CHECK(vkEndCommandBuffer(engine.commandBuffers[engine.currentFrameIndex]),
			"Failed to record command buffer");

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore wait_semaphores[] = { engine.imageAvailableSemaphores[engine.currentFrameIndex] };
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = wait_semaphores;
		submitInfo.pWaitDstStageMask = wait_stages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &engine.commandBuffers[engine.currentFrameIndex];

		VkSemaphore signal_sempahores[] = { engine.renderFinishedSemaphores[engine.currentFrameIndex] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signal_sempahores;

		VIVIUM_VK_CHECK(vkQueueSubmit(engine.graphicsQueue, 1, &submitInfo, engine.inFlightFences[engine.currentFrameIndex]),
			"Failed to submit draw command to buffer");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signal_sempahores;

		VkSwapchainKHR swapChains[] = { window.swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &engine.currentImageIndex;

		VkResult queue_present_result = vkQueuePresentKHR(engine.presentQueue, &presentInfo);

		if (queue_present_result == VK_ERROR_OUT_OF_DATE_KHR
			|| queue_present_result == VK_SUBOPTIMAL_KHR
			|| window.wasFramebufferResized) {

			window.wasFramebufferResized = false;
			_recreateSwapChain(window, engine);
		}
		else if (queue_present_result != VK_SUCCESS) {
			VIVIUM_LOG(Log::FATAL, "Failed to present swap chain image");
		}

		engine.currentFrameIndex = (engine.currentFrameIndex + 1) % Engine::MAX_FRAMES_IN_FLIGHT;

		_checkPerformance(engine);
		_limitFramerate(engine);
	}
		
	void engineBeginRender(Engine& engine, Window& window)
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = engine.renderPass;
		renderPassInfo.framebuffer = window.swapChainFramebuffers[engine.currentImageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = window.swapChainExtent;

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		VkCommandBuffer& commandBuffer = engine.commandBuffers[engine.currentFrameIndex];

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(window.swapChainExtent.width);
		viewport.height = static_cast<float>(window.swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = window.swapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}
		
	void engineEndRender(Engine& engine)
	{
		vkCmdEndRenderPass(engine.commandBuffers[engine.currentFrameIndex]);
	}
}