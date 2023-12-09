#pragma once

#include <vector>

#include "core.h"
#include "math/vec2.h"
#include "allocator.h"

namespace Vivium {
	namespace Engine {
		struct Resource;
	}

	namespace Window {
		struct Options {
			I32x2 dimensions;
			const char* title;
			int multisampleCount;

			Options()
				: dimensions({ 400, 600 }), title("Vivium4"), multisampleCount(2)
			{}
		};

		// TODO: probably best to make vulkan window resource separate from main window?
		class Resource {
		private:
			struct SwapChainSupportDetails {
				VkSurfaceCapabilitiesKHR capabilities;
				std::vector<VkSurfaceFormatKHR> formats;
				std::vector<VkPresentModeKHR> presentModes;
			};

			// GLFW window
			GLFWwindow* m_glfw;

			// Vulkan window
			VkSurfaceKHR m_surface;
			VkSwapchainKHR m_swapChain;
			std::vector<VkImage> m_swapChainImages;
			std::vector<VkImageView> m_swapChainImageViews;
			VkFormat m_swapChainFormat;
			VkExtent2D m_swapChainExtent;
			std::vector<VkFramebuffer> m_swapChainFramebuffers;

			VkImage m_multisampleColorImage;
			VkImageView m_multisampleColorImageView;
			VkDeviceMemory m_multisampleColorMemory;
			VkSampleCountFlagBits m_multisampleCount;

			bool m_wasFramebufferResized;

			// TODO: more

			// General
			I32x2 m_dimensions;

			void m_querySwapChainSupport();

		public:
			void setTitle(const std::string& name);
			void setDimensions(I32x2 dimensions);

			I32x2 getDimensions() const;

			bool isNull() const;
			void close();

			friend Engine::Resource;
		};

		typedef Resource* Handle;

		template <typename AllocatorBase>
		Handle create(Allocator<AllocatorBase>* allocator);
		template <typename AllocatorBase>
		void close(Handle handle, Allocator<AllocatorBase>* allocator);
	}
}