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
			Engine::Resource::SwapChainSupportDetails swap_chain_support = engine->querySwapChainSupport(engine->physicalDevice, this);

			VkSurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(swap_chain_support.formats);
			VkPresentModeKHR present_mode = chooseSwapPresentMode(swap_chain_support.present_modes);
			VkExtent2D extent = chooseSwapExtent(swap_chain_support.capabilities);

			uint32_t imageCount = swap_chain_support.capabilities.minImageCount + 1;
			if (swap_chain_support.capabilities.maxImageCount > 0 && imageCount > swap_chain_support.capabilities.maxImageCount) {
				imageCount = swap_chain_support.capabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = surface;

			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surface_format.format;
			createInfo.imageColorSpace = surface_format.colorSpace;
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

			createInfo.preTransform = swap_chain_support.capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = present_mode;
			createInfo.clipped = VK_TRUE;

			createInfo.oldSwapchain = VK_NULL_HANDLE;

			VIVIUM_VK_CHECK(vkCreateSwapchainKHR(engine->device, &createInfo, nullptr, &m_swap_chain),
				"Failed to create swap chain");

			vkGetSwapchainImagesKHR(engine->device, swapChain, &imageCount, nullptr);
			swapChainImages.resize(imageCount);
			vkGetSwapchainImagesKHR(engine->device, swapChain, &imageCount, swapChainImages.data());

			swapChainImageFormat = surface_format.format;
			swapChainExtent = extent;
		}

		void Resource::createImageViews(Engine::Handle engine)
		{
		}

		void Resource::createMultisampleColorImages(Engine::Handle engine)
		{
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