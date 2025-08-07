#pragma once

#include <array>
#include <vector>

#include "core.h"
#include "math/vec2.h"
#include "storage.h"

namespace Vivium {
	struct Engine;

	struct CommandContext;

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

		VkCommandPool commandPool;
		std::array<VkCommandBuffer, VIVIUM_FRAMES_IN_FLIGHT> commandBuffers;

		std::array<VkSemaphore, VIVIUM_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
		std::array<VkSemaphore, VIVIUM_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
		std::array<VkFence, VIVIUM_FRAMES_IN_FLIGHT> inFlightFences;

		uint32_t currentImageIndex;
		uint32_t currentFrameIndex;

		VkImage multisampleColorImage;
		VkImageView multisampleColorImageView;
		VkDeviceMemory multisampleColorMemory;
		VkSampleCountFlagBits multisampleCount;

		VkRenderPass renderPass;
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
	void _createVulkanObjects(Window& window, Engine& engine);

	void _createWindowCommandPool(Window& window, Engine& engine);
	void _createWindowCommandBuffers(Window& window, Engine& engine);

	void _createSyncObjects(Window& window, Engine& engine);

	/*! \brief Construct a window with the given settings.
	* 
	* Constructs the swapchain and synchronisation objects required for rendering.
	* 
	* \param options Options for the window.
	* \param engine The engine to create the window for.
	* \return The window instance.
	*/
	Window createWindow(WindowOptions const& options, Engine& engine);
	/*! \brief Free window instance and its objects.
	* 
	* \param window Window to free.
	* \param engine Engine used to free objects of window.
	*/
	void dropWindow(Window& window, Engine& engine);

	/*! \brief Get dimensions of window in pixels. */
	I32x2 windowDimensions(Window& window);
	/*! \brief Returns true if window has not been closed. */
	bool windowIsOpen(Window& window, Engine& engine);

	/*! \brief Begin processing of the next frame. */
	void windowBeginFrame(Window& window, CommandContext& context, Engine& engine);
	/*! \brief End processing of the frame. */
	void windowEndFrame(Window& window, Engine& engine);

	/*! \brief Begin rendering of the next frame */
	void windowBeginRender(Window& window);
	/*! \brief End rendering of the next frame */
	void windowEndRender(Window& window);
}