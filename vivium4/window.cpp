#include "window.h"
#include "engine.h"
#include "graphics/primitives/framebuffer.h"

namespace Vivium {
	namespace Window {
		Options::Options()
			: dimensions({ 600, 400 }), title("Vivium4"), multisampleCount(VK_SAMPLE_COUNT_4_BIT)
		{}

		void Resource::framebufferResizeCallback(GLFWwindow* glfwWindow, int width, int height)
		{
			Handle window = reinterpret_cast<Resource*>(glfwGetWindowUserPointer(glfwWindow));

			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(window, Window::isNull);

			window->wasFramebufferResized = true;
			// NOTE: not necessary really
			window->dimensions = I32x2(width, height);
		}

		void Resource::createSwapChain(Engine::Handle engine)
		{
			Engine::Resource::SwapChainSupportDetails swapChainSupport = engine->querySwapChainSupport(engine->physicalDevice, this);

			VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
			VkPresentModeKHR present_mode = chooseSwapPresentMode(swapChainSupport.presentModes);
			VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

			uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
			if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
				imageCount = swapChainSupport.capabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = surface;

			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			Engine::Resource::QueueFamilyIndices indices = engine->findQueueFamilies(engine->physicalDevice, this);
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

			VIVIUM_VK_CHECK(vkCreateSwapchainKHR(engine->device, &createInfo, nullptr, &swapChain), "Failed to create swap chain");

			vkGetSwapchainImagesKHR(engine->device, swapChain, &imageCount, nullptr);
			swapChainImages.resize(imageCount);
			vkGetSwapchainImagesKHR(engine->device, swapChain, &imageCount, swapChainImages.data());

			swapChainImageFormat = surfaceFormat.format;
			swapChainExtent = extent;
		}

		void Resource::createImageViews(Engine::Handle engine)
		{
			swapChainImageViews.resize(swapChainImages.size());

			for (int i = 0; i < swapChainImages.size(); i++) {
				VkImageViewCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				createInfo.image = swapChainImages[i];
				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				createInfo.format = swapChainImageFormat;
				createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				createInfo.subresourceRange.baseMipLevel = 0;
				createInfo.subresourceRange.levelCount = 1;
				createInfo.subresourceRange.baseArrayLayer = 0;
				createInfo.subresourceRange.layerCount = 1;

				VIVIUM_VK_CHECK(vkCreateImageView(engine->device, &createInfo, nullptr, &swapChainImageViews[i]),
					"Failed to create image views");
			}
		}

		void Resource::createMultisampleColorImages(Engine::Handle engine)
		{
			if (multisampleCount & VK_SAMPLE_COUNT_1_BIT) {
				multisampleColorMemory = VK_NULL_HANDLE;
				multisampleColorImage = VK_NULL_HANDLE;
				multisampleColorImageView = VK_NULL_HANDLE;
			}
			else {
				VkImageCreateInfo imageCreateInfo{};

				imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
				imageCreateInfo.extent.width = swapChainExtent.width;
				imageCreateInfo.extent.height = swapChainExtent.height;
				imageCreateInfo.extent.depth = 1;
				imageCreateInfo.mipLevels = 1;
				imageCreateInfo.arrayLayers = 1;
				imageCreateInfo.format = swapChainImageFormat;
				imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageCreateInfo.usage =
					VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				imageCreateInfo.samples = multisampleCount;
				imageCreateInfo.flags = 0;

				VIVIUM_VK_CHECK(vkCreateImage(
					engine->device,
					&imageCreateInfo,
					nullptr,
					&multisampleColorImage
				), "Failed to create image"
				);

				VkMemoryRequirements memoryRequirements;
				vkGetImageMemoryRequirements(
					engine->device,
					multisampleColorImage,
					&memoryRequirements
				);

				VkMemoryAllocateInfo memoryAllocateInfo{};
				memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				memoryAllocateInfo.allocationSize = memoryRequirements.size;
				memoryAllocateInfo.memoryTypeIndex = engine->findMemoryType(
					memoryRequirements.memoryTypeBits,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
				);

				vkAllocateMemory(engine->device,
					&memoryAllocateInfo,
					nullptr,
					&multisampleColorMemory);

				// TODO: only 1 image per allocator as is
				vkBindImageMemory(engine->device, multisampleColorImage, multisampleColorMemory, 0);

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
				viewCreateInfo.image = multisampleColorImage;
				viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewCreateInfo.format = swapChainImageFormat;
				viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				viewCreateInfo.subresourceRange.baseMipLevel = 0;
				viewCreateInfo.subresourceRange.levelCount = 1;
				viewCreateInfo.subresourceRange.baseArrayLayer = 0;
				viewCreateInfo.subresourceRange.layerCount = 1;

				VIVIUM_VK_CHECK(vkCreateImageView(
					engine->device,
					&viewCreateInfo,
					nullptr,
					&multisampleColorImageView),
					"Failed to create image view"
				);
			}
		}

		void Resource::createRenderPass(Engine::Handle engine)
		{
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = swapChainImageFormat;
			colorAttachment.samples = multisampleCount;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = multisampleCount & VK_SAMPLE_COUNT_1_BIT ?
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription colorAttachmentResolve{};
			colorAttachmentResolve.format = swapChainImageFormat;
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

			if (!(multisampleCount & VK_SAMPLE_COUNT_1_BIT)) {
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
			renderPassInfo.attachmentCount = multisampleCount & VK_SAMPLE_COUNT_1_BIT ? 1 : 2;
			renderPassInfo.pAttachments = attachments;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			VIVIUM_VK_CHECK(vkCreateRenderPass(engine->device, &renderPassInfo, nullptr, &engine->renderPass),
				"Failed to create render pass");
		}

		void Resource::createFramebuffers(Engine::Handle engine)
		{
			swapChainFramebuffers.resize(swapChainImages.size());

			for (uint32_t i = 0; i < swapChainImages.size(); i++) {
				std::vector<VkImageView> attachments;

				if (!(multisampleCount & VK_SAMPLE_COUNT_1_BIT)) {
					attachments.push_back(multisampleColorImageView);
				}

				attachments.push_back(swapChainImageViews[i]);

				VkFramebufferCreateInfo framebuffer_info{};
				framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebuffer_info.renderPass = engine->renderPass;
				framebuffer_info.attachmentCount = attachments.size();
				framebuffer_info.pAttachments = attachments.data();
				framebuffer_info.width = swapChainExtent.width;
				framebuffer_info.height = swapChainExtent.height;
				framebuffer_info.layers = 1;

				VIVIUM_VK_CHECK(vkCreateFramebuffer(
					engine->device, &framebuffer_info, nullptr, &swapChainFramebuffers[i]), "Failed to create framebuffer");
			}
		}

		void Resource::deleteSwapChain(Engine::Handle engine)
		{
			if (!(multisampleCount & VK_SAMPLE_COUNT_1_BIT)) {
				vkDestroyImage(engine->device, multisampleColorImage, nullptr);
				vkDestroyImageView(engine->device, multisampleColorImageView, nullptr);
				vkFreeMemory(engine->device, multisampleColorMemory, nullptr);
			}

			for (VkFramebuffer framebuffer : swapChainFramebuffers) {
				vkDestroyFramebuffer(engine->device, framebuffer, nullptr);
			}

			for (VkImageView imageView : swapChainImageViews) {
				vkDestroyImageView(engine->device, imageView, nullptr);
			}

			vkDestroySwapchainKHR(engine->device, swapChain, nullptr);
		}

		void Resource::recreateSwapChain(Engine::Handle engine)
		{
			int width = 0;
			int height = 0;
			glfwGetFramebufferSize(glfwWindow, &width, &height);

			while (width == 0 || height == 0) {
				glfwGetFramebufferSize(glfwWindow, &width, &height);
				glfwWaitEvents();
			}

			vkDeviceWaitIdle(engine->device);

			deleteSwapChain(engine);

			createSwapChain(engine);
			createImageViews(engine);
			createMultisampleColorImages(engine);
			createFramebuffers(engine);
		}

		VkSurfaceFormatKHR Resource::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
		{
			for (const auto& availableFormat : availableFormats) {
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
					return availableFormat;
				}
			}

			return availableFormats[0];
		}

		VkPresentModeKHR Resource::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
		{
			for (const auto& availablePresentMode : availablePresentModes) {
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
					return availablePresentMode;
				}
			}

			return VK_PRESENT_MODE_FIFO_KHR;
		}

		VkExtent2D Resource::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
		{
			if (capabilities.currentExtent.width != UINT_MAX) {
				return capabilities.currentExtent;
			}
			
			int width, height;
			glfwGetFramebufferSize(glfwWindow, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}

		void Resource::setOptions(const Options& options)
		{
			// TODO: might need to handle some re-creation of swap chain,
			//	but should happen automatically regardless since we change window size
			//	unless GLFW is smart and doesn't invoke a callback if window size was
			//	changed to same thing

			glfwSetWindowSize(glfwWindow, options.dimensions.x, options.dimensions.y);
			glfwSetWindowTitle(glfwWindow, options.title);

			dimensions = options.dimensions;
			multisampleCount = static_cast<VkSampleCountFlagBits>(options.multisampleCount);
		}

		void Resource::createSurface(Engine::Handle engine)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);

			VIVIUM_VK_CHECK(glfwCreateWindowSurface(engine->instance, glfwWindow, nullptr, &surface),
				"Failed to create window surface");
		}

		void Resource::initVulkan(Engine::Handle engine)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);

			multisampleCount = static_cast<VkSampleCountFlagBits>(Framebuffer::getRequestedMultisamples(engine, multisampleCount));

			createSwapChain(engine);
			createImageViews(engine);
			if (!(multisampleCount & VK_SAMPLE_COUNT_1_BIT))
				createMultisampleColorImages(engine);
			createRenderPass(engine);
			createFramebuffers(engine);
		}

		bool Resource::isOpen(Engine::Handle engine) const
		{
			bool should_close = glfwWindowShouldClose(glfwWindow);

			// After is_running is false, we assume user will start doing cleanup
			// on objects, so wait for device
			if (should_close)
				vkDeviceWaitIdle(engine->device);

			return !should_close;
		}

		void Resource::create(const Options& options)
		{
			if (!glfwInit()) {
				VIVIUM_LOG(Log::FATAL, "GLFW failed to initialise");
			}

			glfwSetErrorCallback([](int code, const char* desc) {
				VIVIUM_LOG(Log::ERROR, "[GLFW {}] {}", code, desc);
				});

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

			glfwWindow = glfwCreateWindow(options.dimensions.x, options.dimensions.y, options.title, NULL, NULL);

			setOptions(options);

			glfwSetWindowUserPointer(glfwWindow, this);
			glfwSetFramebufferSizeCallback(glfwWindow, Resource::framebufferResizeCallback);
		}

		Resource::Resource()
			:
			glfwWindow(nullptr),
			surface(VK_NULL_HANDLE),
			swapChain(VK_NULL_HANDLE),
			swapChainImageFormat(VkFormat::VK_FORMAT_UNDEFINED),
			multisampleColorImage(VK_NULL_HANDLE),
			multisampleColorImageView(VK_NULL_HANDLE),
			multisampleColorMemory(VK_NULL_HANDLE),
			multisampleCount(VkSampleCountFlagBits::VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM),
			wasFramebufferResized(false),
			dimensions(I32x2(0, 0))
		{}
		
		bool isOpen(Window::Handle window, Engine::Handle engine)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(window, Window::isNull);
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);

			return window->isOpen(engine);
		}

		bool isNull(const Handle handle)
		{
			return handle->surface == VK_NULL_HANDLE;
		}

		I32x2 dimensions(Window::Handle window)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(window, Window::isNull);

			return window->dimensions;
		}
	}
}