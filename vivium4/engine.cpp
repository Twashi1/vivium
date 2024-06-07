#include "engine.h"

namespace Vivium {
	namespace Engine {
		void Resource::populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
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

			if (severity == Log::ERROR) {
				int x = 3;
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

		bool Resource::isSuitableDevice(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions, Window::Handle window)
		{
			QueueFamilyIndices queueFamilyIndices = findQueueFamilies(device, window);

			// TODO: shouldn'y be all extensions, just device ones
			bool extensionSupport = checkDeviceExtensionSupport(deviceExtensions, device);
			bool swapChainAdequate = false;

			VkPhysicalDeviceFeatures supportedFeatures;
			vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

			if (extensionSupport) {
				SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, window);
				swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
			}

			return queueFamilyIndices.isComplete() && extensionSupport && swapChainAdequate && supportedFeatures.samplerAnisotropy;
		}
		
		bool Resource::checkDeviceExtensionSupport(const std::vector<const char*>& requiredExtensions, VkPhysicalDevice device)
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

		void Resource::setOptions(const Options& options)
		{
			VIVIUM_ASSERT(options.fps > 0, "Can't have 0fps");

			targetTimePerFrame = 1.0f / options.fps;
			pollPeriod = options.pollPeriod;
		}

		void Resource::pickPhysicalDevice(const std::vector<const char*>& deviceExtensions, Window::Handle window)
		{
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

			if (deviceCount == 0) {
				VIVIUM_LOG(Log::FATAL, "Couldn't find GPU with vulkan support");
			}
			
			VIVIUM_LOG(Log::DEBUG, "Found {} devices", deviceCount);

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

			// TODO: change back
			for (uint64_t deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++) {
				VkPhysicalDevice device = devices[deviceIndex];

				if (isSuitableDevice(device, deviceExtensions, window)) {
					physicalDevice = device;

					VIVIUM_LOG(Log::DEBUG, "Selected device {}", deviceIndex);

					break;
				}
			}

			VIVIUM_ASSERT(physicalDevice != VK_NULL_HANDLE, "Failed to find suitable GPU");
		}

		void Resource::createLogicalDevice(Window::Handle window, const std::span<const char* const> extensions, const std::span<const char* const> validationLayers)
		{
			QueueFamilyIndices indices = findQueueFamilies(physicalDevice, window);

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

			if (enableValidationLayers) {
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();
			}
			else {
				createInfo.enabledLayerCount = 0;
			}

			VIVIUM_VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device),
				"Failed to create logical device");

			vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
			vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
			vkGetDeviceQueue(device, indices.transferFamily, 0, &transferQueue);
		}

		void Resource::createCommandPool(Window::Handle window)
		{
			QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, window);

			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

			VIVIUM_VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool),
				"Failed to create command pool");
		}

		void Resource::createCommandBuffers()
		{
			commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

			VkCommandBufferAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocateInfo.commandPool = commandPool;
			allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

			VIVIUM_VK_CHECK(vkAllocateCommandBuffers(device, &allocateInfo, commandBuffers.data()),
				"Failed to allocate command buffers");
		}

		void Resource::createSyncObjects()
		{
			imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
			renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
			inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				// TODO: VkCheck this
				if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
					vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
					vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
					VIVIUM_LOG(Log::FATAL, "Failed to create sync objects for a frame");
				}
			}
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

			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();

			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

			if (enableValidationLayers) {
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
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
			VkResult returnValue;

			if (func != nullptr) {
				returnValue = func(instance, &createInfo, nullptr, &debugMessenger);
			}
			else {
				returnValue = VK_ERROR_EXTENSION_NOT_PRESENT;
			}

			VIVIUM_VK_CHECK(returnValue, "Failed to create debug messenger");
		}

		uint32_t Resource::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
		{
			VkPhysicalDeviceMemoryProperties memoryProperties;
			vkGetPhysicalDeviceMemoryProperties(
				physicalDevice,
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

		void Resource::checkPerformance()
		{
			float maxTimeSpent = targetTimePerFrame * pollFramesCounted;

			if (maxTimeSpent > pollPeriod) {
				float calculatedTimePerFrame = pollFramesElapsedTime / pollFramesCounted;
				float calculatedTimePerUpdate = pollUpdatesElapsedTime / pollFramesCounted;

				VIVIUM_LOG(Log::DEBUG,
					"Max FPS: {}, True FPS: {}, TPF: {}ms",
					1.0f / calculatedTimePerFrame,
					1.0f / calculatedTimePerUpdate,
					calculatedTimePerFrame * 1000.0f
				);

				float delta_tpf = calculatedTimePerFrame - targetTimePerFrame;

				if (delta_tpf > 0.0f)
					VIVIUM_LOG(Log::WARN, "Running behind by {}ms on average",
						delta_tpf * 1000.0f);

				pollFramesCounted = 0;
				pollFramesElapsedTime = 0.0f;
				pollUpdatesElapsedTime = 0.0f;
			}
		}

		void Resource::limitFramerate()
		{
			float elapsed = frameTimer.getTime();

			pollFramesElapsedTime += elapsed;
			++pollFramesCounted;

			float sleep_time = targetTimePerFrame - elapsed;

			if (sleep_time > 0.0f) {
				Time::nanosleep(static_cast<long long>(static_cast<double>(sleep_time) * 1.0E9));
			}

			frameTimer.reset();
			pollUpdatesElapsedTime += updateTimer.reset();
		}

		void Resource::create(Options options, Window::Handle window)
		{
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

			// TODO: dont need return value
			std::vector<const char*> extensions = createInstance(validationLayers, defaultExtensions);
			setupDebugMessenger();

			window->createSurface(this);

			pickPhysicalDevice(deviceExtensions, window);

			setOptions(options);

			createLogicalDevice(window, deviceExtensions, validationLayers);

			window->initVulkan(this);

			createCommandPool(window);
			createCommandBuffers();

			createSyncObjects();
		}

		void Resource::drop(Window::Handle window) {
			vkDeviceWaitIdle(device);

			vkDestroyRenderPass(device, renderPass, nullptr);

			for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
				vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
				vkDestroyFence(device, inFlightFences[i], nullptr);
			}

			vkDestroyCommandPool(device, commandPool, nullptr);

			vkDestroyDevice(device, nullptr);

			if (enableValidationLayers) {
				auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
				
				if (func != nullptr) {
					func(instance, debugMessenger, nullptr);
				}
			}

			vkDestroySurfaceKHR(instance, window->surface, nullptr);
			vkDestroyInstance(instance, nullptr);

			glfwTerminate();
		}
		
		void Resource::beginFrame(Window::Handle window)
		{
			glfwPollEvents();
			vkWaitForFences(device, 1, &inFlightFences[currentFrameIndex], VK_TRUE, UINT64_MAX);

			VkResult acquireImageResult = vkAcquireNextImageKHR(
				device,
				window->swapChain,
				UINT64_MAX,
				imageAvailableSemaphores[currentFrameIndex],
				VK_NULL_HANDLE,
				&currentImageIndex
			);

			if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
				window->recreateSwapChain(this);

				return;
			}
			else if (acquireImageResult != VK_SUCCESS && acquireImageResult != VK_SUBOPTIMAL_KHR) {
				VIVIUM_LOG(Log::FATAL, "Failed to acquire swapchain image");
			}

			vkResetFences(device, 1, &inFlightFences[currentFrameIndex]);

			vkResetCommandBuffer(commandBuffers[currentFrameIndex], 0);

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			VIVIUM_VK_CHECK(vkBeginCommandBuffer(commandBuffers[currentFrameIndex], &beginInfo),
				"Failed to begin recording command buffer");
		}
		
		void Resource::endFrame(Window::Handle window)
		{
			// TODO: fix variable naming style conventions

			VIVIUM_VK_CHECK(vkEndCommandBuffer(commandBuffers[currentFrameIndex]),
				"Failed to record command buffer");

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore wait_semaphores[] = { imageAvailableSemaphores[currentFrameIndex] };
			VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = wait_semaphores;
			submitInfo.pWaitDstStageMask = wait_stages;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffers[currentFrameIndex];

			VkSemaphore signal_sempahores[] = { renderFinishedSemaphores[currentFrameIndex] };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signal_sempahores;

			VIVIUM_VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrameIndex]),
				"Failed to submit draw command to buffer");

			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signal_sempahores;

			VkSwapchainKHR swapChains[] = { window->swapChain };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;

			presentInfo.pImageIndices = &currentImageIndex;

			VkResult queue_present_result = vkQueuePresentKHR(presentQueue, &presentInfo);

			if (queue_present_result == VK_ERROR_OUT_OF_DATE_KHR
				|| queue_present_result == VK_SUBOPTIMAL_KHR
				|| window->wasFramebufferResized) {

				window->wasFramebufferResized = false;
				window->recreateSwapChain(this);
			}
			else if (queue_present_result != VK_SUCCESS) {
				VIVIUM_LOG(Log::FATAL, "Failed to present swap chain image");
			}

			currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

			checkPerformance();
			limitFramerate();
		}
		
		void Resource::beginRender(Window::Handle window)
		{
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = window->swapChainFramebuffers[currentImageIndex];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = window->swapChainExtent;

			VkClearValue clear_color = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clear_color;

			VkCommandBuffer& commandBuffer = commandBuffers[currentFrameIndex];

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(window->swapChainExtent.width);
			viewport.height = static_cast<float>(window->swapChainExtent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = window->swapChainExtent;
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		}
		
		void Resource::endRender()
		{
			vkCmdEndRenderPass(commandBuffers[currentFrameIndex]);
		}

		Resource::Resource()
			:
			instance(VK_NULL_HANDLE),
			debugMessenger(VK_NULL_HANDLE),
			physicalDevice(VK_NULL_HANDLE),
			device(VK_NULL_HANDLE),
			renderPass(VK_NULL_HANDLE),
			commandPool(VK_NULL_HANDLE),
			graphicsQueue(VK_NULL_HANDLE),
			presentQueue(VK_NULL_HANDLE),
			transferQueue(VK_NULL_HANDLE),
			currentFrameIndex(0),
			currentImageIndex(UINT32_MAX),
			targetTimePerFrame(0.0f),
			pollPeriod(0.0f),
			pollFramesElapsedTime(0.0f),
			pollUpdatesElapsedTime(0.0f),
			pollFramesCounted(0)
		{}
		
		bool isNull(const Engine::Handle handle)
		{
			return handle->instance == VK_NULL_HANDLE;
		}

		void beginFrame(Engine::Handle engine, Window::Handle window)
		{
			engine->beginFrame(window);
		}
		
		void endFrame(Engine::Handle engine, Window::Handle window)
		{
			engine->endFrame(window);
		}
		
		void beginRender(Engine::Handle engine, Window::Handle window)
		{
			engine->beginRender(window);
		}
		
		void endRender(Engine::Handle engine)
		{
			engine->endRender();
		}
	}
}