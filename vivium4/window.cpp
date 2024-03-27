#include "window.h"
#include "engine.h"

namespace Vivium {
	namespace Window {
		Options::Options()
			: dimensions({ 400, 600 }), title("Vivium4"), multisampleCount(2)
		{}

		bool Resource::isNull() const
		{
			return surface == VK_NULL_HANDLE;
		}

		void Resource::close()
		{
			// TODO
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

			VIVIUM_VK_CHECK(vkCreateSwapchainKHR(engine->device, &createInfo, nullptr, &swapChain),
				"Failed to create swap chain");

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

		void Resource::createSurface(Engine::Handle engine)
		{
			VIVIUM_CHECK_RESOURCE(engine);

			VIVIUM_VK_CHECK(glfwCreateWindowSurface(engine->instance, glfwWindow, nullptr, &surface),
				"Failed to create window surface");
		}

		void Resource::initVulkan(Engine::Handle engine)
		{
			VIVIUM_CHECK_RESOURCE(engine);

			createSwapChain(engine);
			createImageViews(engine);
			createMultisampleColorImages(engine);
		}

		I32x2 Resource::getDimensions() const
		{
			return dimensions;
		}
	}
}