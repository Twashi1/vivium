#pragma once

#include "allocator.h"
#include "core.h"
#include "error/log.h"
#include "window.h"

namespace Vivium {
	namespace Engine {
		class Resource {
		private:
			static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
			static constexpr bool m_enableValidationLayers = VIVIUM_IS_DEBUG;

			struct QueueFamilyIndices {
				uint32_t graphicsFamily, presentFamily, transferFamily;

				bool isComplete() const { return graphicsFamily != UINT_MAX && presentFamily != UINT_MAX && transferFamily != UINT_MAX; }
			};

			VkRenderPass m_renderPass;
			VkCommandPool m_commandPool;

			std::vector<VkCommandBuffer> m_commandBuffers;

			std::vector<VkSemaphore> m_imageAvailableSemaphores;
			std::vector<VkSemaphore> m_renderFinishedSemaphores;
			std::vector<VkFence> m_inFlightFences;

			uint32_t m_currentFrameIndex;
			uint32_t m_currentImageIndex;

			static VKAPI_ATTR VkBool32 VKAPI_CALL m_vulkanDebugCallback(
				VkDebugUtilsMessageSeverityFlagBitsEXT vkSeverity,
				VkDebugUtilsMessageTypeFlagsEXT messageType,
				const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
				void* userData
			);

			QueueFamilyIndices m_findQueueFamilies(VkPhysicalDevice device, Window::Handle window);

			bool m_checkValidationLayerSupport(const std::vector<const char*>& validationLayers);
			bool m_isSuitableDevice(VkPhysicalDevice device, const std::vector<const char*>& requiredExtensions, Window::Handle window);
			bool m_checkDeviceExtensionSupport(const std::vector<const char*>& requiredExtensions, VkPhysicalDevice device);


		public:
			bool isNull() const;
			void close();
		};

		typedef Resource* Handle;

		template <typename AllocatorBase>
		void create(Allocator<AllocatorBase>* allocator);
		template <typename AllocatorBase>
		void close(Handle handle, Allocator<AllocatorBase>* allocator);
	}
}