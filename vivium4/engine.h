#pragma once

#include <array>
#include <span>

#include "storage.h"
#include "core.h"
#include "error/log.h"
#include "window.h"

namespace Vivium {
	namespace Engine {
		struct Options {
			
		};

		struct Resource {
			static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
			static constexpr bool enableValidationLayers = VIVIUM_IS_DEBUG;

			struct SwapChainSupportDetails {
				VkSurfaceCapabilitiesKHR capabilities;
				std::vector<VkSurfaceFormatKHR> formats;
				std::vector<VkPresentModeKHR> presentModes;
			};

			struct QueueFamilyIndices {
				uint32_t graphicsFamily, presentFamily, transferFamily;

				bool isComplete() const { return graphicsFamily != UINT_MAX && presentFamily != UINT_MAX && transferFamily != UINT_MAX; }
			};

			VkInstance instance;
			VkDebugUtilsMessengerEXT debugMessenger;

			VkPhysicalDevice physicalDevice;
			VkDevice device;

			VkRenderPass renderPass;
			VkCommandPool commandPool;

			std::vector<VkCommandBuffer> commandBuffers;

			std::vector<VkSemaphore> imageAvailableSemaphores;
			std::vector<VkSemaphore> renderFinishedSemaphores;
			std::vector<VkFence> inFlightFences;

			uint32_t currentFrameIndex;
			uint32_t currentImageIndex;

			void populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT createInfo);

			static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
				VkDebugUtilsMessageSeverityFlagBitsEXT vkSeverity,
				VkDebugUtilsMessageTypeFlagsEXT messageType,
				const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
				void* userData
			);

			// TODO: consider moving to window?
			QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, Window::Handle window);

			bool checkValidationLayerSupport(const std::span<const char* const>& validationLayers);
			bool isSuitableDevice(VkPhysicalDevice device, const std::vector<const char*>& requiredExtensions, Window::Handle window);
			bool checkDeviceExtensionSupport(const std::vector<const char*>& requiredExtensions, VkPhysicalDevice device);
			
			// TODO: consider moving to window
			SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, Window::Handle window);

			void pickPhysicalDevice(const std::vector<const char*>& extensions, Window::Handle window);
			void createLogicalDevice();

			void createRenderPass();

			void createFramebuffers();

			void createCommandPool();
			void createCommandBuffers();

			void createSyncObjects();

			std::vector<const char*> createInstance(const std::span<const char* const> validationLayers, const std::span<const char* const> defaultExtensions);
			void setupDebugMessenger();

			void create(Options options, Window::Handle window);

			bool isNull() const;
			void close();
		};

		typedef Resource* Handle;

		template <Allocator::AllocatorType Storage>
		Handle create(Storage storage, Options options)
		{
			return storage.getHandle<Resource>(options);
		}

		template <Allocator::AllocatorType Storage>
		void close(Storage storage, Handle handle)
		{
			return storage.freeHandle<Resource>(handle);
		}
	}
}