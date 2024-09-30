#include "window.h"
#include "engine.h"
#include "graphics/primitives/framebuffer.h"
#include "graphics/commands.h"

namespace Vivium {
	void dropWindow(Window& window, Engine& engine)
	{
		_deleteSwapChain(window, engine);

		vkDestroyCommandPool(engine.device, window.commandPool, nullptr);

		for (uint32_t i = 0; i < VIVIUM_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(engine.device, window.renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(engine.device, window.imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(engine.device, window.inFlightFences[i], nullptr);
		}

		vkDestroyRenderPass(engine.device, window.renderPass, nullptr);
		vkDestroySurfaceKHR(engine.instance, window.surface, nullptr);
	}

	void _createSwapChain(Window& window, Engine& engine)
	{
		Engine::SwapChainSupportDetails swapChainSupport = _querySwapChainSupport(engine.physicalDevice, window);

		VkSurfaceFormatKHR surfaceFormat = _chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR present_mode = _chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = _chooseSwapExtent(window, swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = window.surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		Engine::QueueFamilyIndices indices = _findQueueFamilies(engine, engine.physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = present_mode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VIVIUM_VK_CHECK(vkCreateSwapchainKHR(engine.device, &createInfo, nullptr, &window.swapChain), "Failed to create swap chain");

		vkGetSwapchainImagesKHR(engine.device, window.swapChain, &imageCount, nullptr);
		window.swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(engine.device, window.swapChain, &imageCount, window.swapChainImages.data());

		window.swapChainImageFormat = surfaceFormat.format;
		window.swapChainExtent = extent;
	}

	void _createImageViews(Window& window, Engine& engine)
	{
		window.swapChainImageViews.resize(window.swapChainImages.size());

		for (int i = 0; i < window.swapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = window.swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = window.swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VIVIUM_VK_CHECK(vkCreateImageView(engine.device, &createInfo, nullptr, &window.swapChainImageViews[i]),
				"Failed to create image views");
		}
	}

	void _createMultisampleColorImages(Window& window, Engine& engine)
	{
		if (window.multisampleCount & VK_SAMPLE_COUNT_1_BIT) {
			window.multisampleColorMemory = VK_NULL_HANDLE;
			window.multisampleColorImage = VK_NULL_HANDLE;
			window.multisampleColorImageView = VK_NULL_HANDLE;
		}
		else {
			VkImageCreateInfo imageCreateInfo{};

			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.extent.width = window.swapChainExtent.width;
			imageCreateInfo.extent.height = window.swapChainExtent.height;
			imageCreateInfo.extent.depth = 1;
			imageCreateInfo.mipLevels = 1;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.format = window.swapChainImageFormat;
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageCreateInfo.usage =
				VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageCreateInfo.samples = window.multisampleCount;
			imageCreateInfo.flags = 0;

			VIVIUM_VK_CHECK(vkCreateImage(
				engine.device,
				&imageCreateInfo,
				nullptr,
				&window.multisampleColorImage
			), "Failed to create image"
			);

			VkMemoryRequirements memoryRequirements;
			vkGetImageMemoryRequirements(
				engine.device,
				window.multisampleColorImage,
				&memoryRequirements
			);

			VkMemoryAllocateInfo memoryAllocateInfo{};
			memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memoryAllocateInfo.allocationSize = memoryRequirements.size;
			memoryAllocateInfo.memoryTypeIndex = _findMemoryType(
				engine,
				memoryRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);

			vkAllocateMemory(engine.device,
				&memoryAllocateInfo,
				nullptr,
				&window.multisampleColorMemory);

			// TODO: only 1 image per allocator as is
			vkBindImageMemory(engine.device, window.multisampleColorImage, window.multisampleColorMemory, 0);

			// TODO: not even needed?
			/*
			tmp_transition(image, command_buffer[0],
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, // TODO: SUBJECT TO CHANGE
				VK_ACCESS_NONE,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
			);
			*/

			// CREATE VIEW
			VkImageViewCreateInfo viewCreateInfo{};
			viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCreateInfo.image = window.multisampleColorImage;
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewCreateInfo.format = window.swapChainImageFormat;
			viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewCreateInfo.subresourceRange.baseMipLevel = 0;
			viewCreateInfo.subresourceRange.levelCount = 1;
			viewCreateInfo.subresourceRange.baseArrayLayer = 0;
			viewCreateInfo.subresourceRange.layerCount = 1;

			VIVIUM_VK_CHECK(vkCreateImageView(
				engine.device,
				&viewCreateInfo,
				nullptr,
				&window.multisampleColorImageView),
				"Failed to create image view"
			);
		}
	}

	void _createRenderPass(Window& window, Engine& engine)
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = window.swapChainImageFormat;
		colorAttachment.samples = window.multisampleCount;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = window.multisampleCount & VK_SAMPLE_COUNT_1_BIT ?
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = window.swapChainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 1;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		if (!(window.multisampleCount & VK_SAMPLE_COUNT_1_BIT)) {
			subpass.pResolveAttachments = &colorAttachmentResolveRef;
		}

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = VK_ACCESS_NONE;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkAttachmentDescription attachments[2] = {
			colorAttachment,
			colorAttachmentResolve // Only include in non-multisampled image
		};

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = window.multisampleCount & VK_SAMPLE_COUNT_1_BIT ? 1 : 2;
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VIVIUM_VK_CHECK(vkCreateRenderPass(engine.device, &renderPassInfo, nullptr, &window.renderPass),
			"Failed to create render pass");
	}

	void _createFramebuffers(Window& window, Engine& engine)
	{
		window.swapChainFramebuffers.resize(window.swapChainImages.size());

		for (uint32_t i = 0; i < window.swapChainImages.size(); i++) {
			std::vector<VkImageView> attachments;

			if (!(window.multisampleCount & VK_SAMPLE_COUNT_1_BIT)) {
				attachments.push_back(window.multisampleColorImageView);
			}

			attachments.push_back(window.swapChainImageViews[i]);

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = window.renderPass;
			framebufferInfo.attachmentCount = attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = window.swapChainExtent.width;
			framebufferInfo.height = window.swapChainExtent.height;
			framebufferInfo.layers = 1;

			VIVIUM_VK_CHECK(vkCreateFramebuffer(
				engine.device, &framebufferInfo, nullptr, &window.swapChainFramebuffers[i]), "Failed to create framebuffer");
		}
	}

	void _deleteSwapChain(Window& window, Engine& engine)
	{
		if (!(window.multisampleCount & VK_SAMPLE_COUNT_1_BIT)) {
			vkDestroyImage(engine.device, window.multisampleColorImage, nullptr);
			vkDestroyImageView(engine.device, window.multisampleColorImageView, nullptr);
			vkFreeMemory(engine.device, window.multisampleColorMemory, nullptr);
		}

		for (VkFramebuffer framebuffer : window.swapChainFramebuffers) {
			vkDestroyFramebuffer(engine.device, framebuffer, nullptr);
		}

		for (VkImageView imageView : window.swapChainImageViews) {
			vkDestroyImageView(engine.device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(engine.device, window.swapChain, nullptr);
	}

	void _recreateSwapChain(Window& window, Engine& engine)
	{
		int width = 0;
		int height = 0;
		glfwGetFramebufferSize(window.glfwWindow, &width, &height);

		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window.glfwWindow, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(engine.device);

		_deleteSwapChain(window, engine);

		_createSwapChain(window, engine);
		_createImageViews(window, engine);
		_createMultisampleColorImages(window, engine);
		_createFramebuffers(window, engine);
	}

	VkSurfaceFormatKHR _chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR _chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D _chooseSwapExtent(Window& window, const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT_MAX) {
			return capabilities.currentExtent;
		}
			
		int width, height;
		glfwGetFramebufferSize(window.glfwWindow, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

	void _setWindowOptions(Window& window, WindowOptions const& options)
	{
		// TODO: might need to handle some re-creation of swap chain,
		//	does GLFW invoke the callback when window size changes?

		glfwSetWindowSize(window.glfwWindow, options.dimensions.x, options.dimensions.y);
		glfwSetWindowTitle(window.glfwWindow, options.title);

		window.multisampleCount = static_cast<VkSampleCountFlagBits>(options.multisampleCount);
	}

	void _createSurface(Window& window, Engine& engine)
	{
		VIVIUM_VK_CHECK(glfwCreateWindowSurface(engine.instance, window.glfwWindow, nullptr, &window.surface),
			"Failed to create window surface");
	}

	void _createVulkanObjects(Window& window, Engine& engine)
	{
		_createSurface(window, engine);

		window.multisampleCount = static_cast<VkSampleCountFlagBits>(getRequestedMultisamples(engine, window.multisampleCount));

		_createSwapChain(window, engine);
		_createImageViews(window, engine);

		if (!(window.multisampleCount & VK_SAMPLE_COUNT_1_BIT))
			_createMultisampleColorImages(window, engine);

		_createRenderPass(window, engine);
		_createFramebuffers(window, engine);

		_createWindowCommandPool(window, engine);
		_createWindowCommandBuffers(window, engine);
		_createSyncObjects(window, engine);
	}

	void _createWindowCommandPool(Window& window, Engine& engine)
	{
		Engine::QueueFamilyIndices queueFamilyIndices = _findQueueFamilies(engine, engine.physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

		VIVIUM_VK_CHECK(vkCreateCommandPool(engine.device, &poolInfo, nullptr, &window.commandPool),
			"Failed to create command pool");
	}

	void _createWindowCommandBuffers(Window& window, Engine& engine)
	{
		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = window.commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = static_cast<uint32_t>(window.commandBuffers.size());

		VIVIUM_VK_CHECK(vkAllocateCommandBuffers(engine.device, &allocateInfo, window.commandBuffers.data()),
			"Failed to allocate command buffers");
	}

	void _createSyncObjects(Window& window, Engine& engine)
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint32_t i = 0; i < VIVIUM_FRAMES_IN_FLIGHT; i++) {
			// TODO: VkCheck this
			if (vkCreateSemaphore(engine.device, &semaphoreInfo, nullptr, &window.imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(engine.device, &semaphoreInfo, nullptr, &window.renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(engine.device, &fenceInfo, nullptr, &window.inFlightFences[i]) != VK_SUCCESS) {
				VIVIUM_LOG(Log::FATAL, "Failed to create sync objects for a frame");
			}
		}
	}

	Window createWindow(WindowOptions const& options, Engine& engine)
	{
		Window window;
		window.glfwWindow = nullptr;
		window.surface = VK_NULL_HANDLE;
		window.swapChain = VK_NULL_HANDLE;
		window.swapChainImageFormat = VK_FORMAT_UNDEFINED;
		window.multisampleColorImage = VK_NULL_HANDLE;
		window.multisampleColorImageView = VK_NULL_HANDLE;
		window.multisampleColorMemory = VK_NULL_HANDLE;
		window.multisampleCount = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
		window.currentFrameIndex = 0;
		window.currentImageIndex = UINT32_MAX;

		window.glfwWindow = glfwCreateWindow(options.dimensions.x, options.dimensions.y, options.title, NULL, NULL);

		_setWindowOptions(window, options);

		glfwSetWindowUserPointer(window.glfwWindow, &window);

		_createVulkanObjects(window, engine);

		return window;
	}

	I32x2 windowDimensions(Window& window)
	{
		I32x2 dimensions;
		glfwGetWindowSize(window.glfwWindow, &dimensions.x, &dimensions.y);

		return dimensions;
	}

	bool windowIsOpen(Window& window, Engine& engine)
	{
		bool shouldClose = glfwWindowShouldClose(window.glfwWindow);

		// After shouldClose is false, we assume user will start doing cleanup
		// on objects, so wait for device
		if (shouldClose)
			vkDeviceWaitIdle(engine.device);

		return !shouldClose;
	}

	void windowBeginRender(Window& window)
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = window.renderPass;
		renderPassInfo.framebuffer = window.swapChainFramebuffers[window.currentImageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = window.swapChainExtent;

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		VkCommandBuffer& commandBuffer = window.commandBuffers[window.currentFrameIndex];

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

	void windowEndRender(Window& window)
	{
		vkCmdEndRenderPass(window.commandBuffers[window.currentFrameIndex]);
	}

	void windowBeginFrame(Window& window, CommandContext& context, Engine& engine)
	{
		vkWaitForFences(engine.device, 1, &window.inFlightFences[window.currentFrameIndex], VK_TRUE, UINT64_MAX);

		VkResult acquireImageResult = vkAcquireNextImageKHR(
			engine.device,
			window.swapChain,
			UINT64_MAX,
			window.imageAvailableSemaphores[window.currentFrameIndex],
			VK_NULL_HANDLE,
			&window.currentImageIndex
		);

		if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
			_recreateSwapChain(window, engine);

			return;
		}
		else if (acquireImageResult != VK_SUCCESS && acquireImageResult != VK_SUBOPTIMAL_KHR) {
			VIVIUM_LOG(Log::FATAL, "Failed to acquire swapchain image");
		}

		vkResetFences(engine.device, 1, &window.inFlightFences[window.currentFrameIndex]);

		vkResetCommandBuffer(window.commandBuffers[window.currentFrameIndex], 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VIVIUM_VK_CHECK(vkBeginCommandBuffer(window.commandBuffers[window.currentFrameIndex], &beginInfo),
			"Failed to begin recording command buffer");

		// Switch context to start recording this command buffer
		context.currentCommandBuffer = window.commandBuffers[window.currentFrameIndex];
	}

	void windowEndFrame(Window& window, Engine& engine)
	{
		VIVIUM_VK_CHECK(vkEndCommandBuffer(window.commandBuffers[window.currentFrameIndex]),
			"Failed to record command buffer");

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { window.imageAvailableSemaphores[window.currentFrameIndex] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &window.commandBuffers[window.currentFrameIndex];

		VkSemaphore signalSemaphores[] = { window.renderFinishedSemaphores[window.currentFrameIndex] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VIVIUM_VK_CHECK(vkQueueSubmit(engine.graphicsQueue, 1, &submitInfo, window.inFlightFences[window.currentFrameIndex]),
			"Failed to submit draw command to buffer");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { window.swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &window.currentImageIndex;

		VkResult queuePresentResult = vkQueuePresentKHR(engine.presentQueue, &presentInfo);

		if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR
			|| queuePresentResult == VK_SUBOPTIMAL_KHR) {
			_recreateSwapChain(window, engine);
		}
		else if (queuePresentResult != VK_SUCCESS) {
			VIVIUM_LOG(Log::FATAL, "Failed to present swap chain image");
		}

		window.currentFrameIndex = (window.currentFrameIndex + 1) % VIVIUM_FRAMES_IN_FLIGHT;
	}
}