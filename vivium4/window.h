#pragma once

#include <vector>

#include "core.h"
#include "math/vec2.h"
#include "storage.h"

namespace Vivium {
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
			VkFormat swapChainFormat;
			VkExtent2D swapChainExtent;
			std::vector<VkFramebuffer> swapChainFramebuffers;

			VkImage multisampleColorImage;
			VkImageView multisampleColorImageView;
			VkDeviceMemory multisampleColorMemory;
			VkSampleCountFlagBits multisampleCount;

			bool wasFramebufferResized;

			// TODO: more

			I32x2 dimensions;

			void createSwapChain(Engine::Handle engine);
			void createImageViews(Engine::Handle engine);
			void createMultisampleColorImages(Engine::Handle engine);

			void setTitle(const std::string& name);
			void setDimensions(I32x2 dimensions);

			void createSurface(Engine::Handle engine);
			void initVulkan(Engine::Handle engine);

			I32x2 getDimensions() const;

			bool isNull() const;
			void close();
		};

		typedef Resource* Handle;

		template <typename Storage>
		Handle create(Storage storage, Options options)
		{
			return storage.getHandle<Resource>(options);
		}

		template <typename Storage>
		void close(Storage storage, Handle handle)
		{
			return storage.freeHandle<Resource>(handle)
		}
	}
}