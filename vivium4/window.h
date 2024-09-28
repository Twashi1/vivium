#pragma once

#include <vector>

#include "core.h"
#include "math/vec2.h"
#include "storage.h"

namespace Vivium {
	struct Engine;

	struct WindowOptions {
		I32x2 dimensions = I32x2(600, 400);
		const char* title = "Vivium4";
		int multisampleCount = VK_SAMPLE_COUNT_4_BIT;
	};

	// TODO: probably best to make vulkan window resource separate from main window?
	struct Window {
		// GLFW window
		GLFWwindow* glfwWindow;

		// Vulkan window
		VkSurfaceKHR surface;
		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VkFramebuffer> swapChainFramebuffers;

		VkImage multisampleColorImage;
		VkImageView multisampleColorImageView;
		VkDeviceMemory multisampleColorMemory;
		VkSampleCountFlagBits multisampleCount;
	};

	void _createSwapChain(Window& window, Engine& engine);
	void _createImageViews(Window& window, Engine& engine);
	void _createMultisampleColorImages(Window& window, Engine& engine);
	void _createRenderPass(Window& window, Engine& engine);
	void _createFramebuffers(Window& window, Engine& engine);

	void _deleteSwapChain(Window& window, Engine& engine);
	void _recreateSwapChain(Window& window, Engine& engine);

	static VkSurfaceFormatKHR _chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR _chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D _chooseSwapExtent(Window& window, const VkSurfaceCapabilitiesKHR& capabilities);

	void _setWindowOptions(Window& window, WindowOptions const& options);

	void _createSurface(Window& window, Engine& engine);
	void _initVulkan(Window& window, Engine& engine);

	Window createWindow(WindowOptions const& options);
	void dropWindow(Window& window, Engine& engine);

	I32x2 windowDimensions(Window& window);
	bool isWindowOpen(Window& window, Engine& engine);
}