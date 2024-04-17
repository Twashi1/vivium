#pragma once

#include <vector>

#include "core.h"
#include "math/vec2.h"
#include "storage.h"

namespace Vivium {
	namespace Engine {
		struct Resource;

		typedef Resource* Handle;
	}

	namespace Window {
		struct Options {
			I32x2 dimensions;
			const char* title;
			int multisampleCount;

			Options();
		};

		// TODO: probably best to make vulkan window resource separate from main window?
		struct Resource {
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

			bool wasFramebufferResized;

			I32x2 dimensions;

			static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

			void createSwapChain(Engine::Handle engine);
			void createImageViews(Engine::Handle engine);
			void createMultisampleColorImages(Engine::Handle engine);
			void createRenderPass(Engine::Handle engine);
			void createFramebuffers(Engine::Handle engine);

			void deleteSwapChain(Engine::Handle engine);
			void recreateSwapChain(Engine::Handle engine);

			static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
			static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
			VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

			void setOptions(const Options& options);

			void createSurface(Engine::Handle engine);
			void initVulkan(Engine::Handle engine);

			bool isOpen(Engine::Handle engine) const;

			void create(const Options& options);

			void drop(Engine::Handle engine);

			Resource();
		};

		typedef Resource* Handle;

		bool isNull(const Handle handle);

		template <Allocator::AllocatorType AllocatorType>
		Handle create(AllocatorType allocator, Options options)
		{
			Handle window = Allocator::allocateResource<Resource>(allocator);

			window->create(options);

			return window;
		}

		template <Allocator::AllocatorType AllocatorType>
		void drop(AllocatorType allocator, Handle handle, Engine::Handle engine)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(handle, Window::isNull);

			handle->drop(engine);
			Allocator::dropResource(allocator, handle);
		}

		I32x2 dimensions(Window::Handle window);
		bool isOpen(Window::Handle window, Engine::Handle engine);
	}
}