#pragma once

#include <array>
#include <set>
#include <span>

#include "storage.h"
#include "core.h"
#include "error/log.h"
#include "window.h"

namespace Vivium {
	namespace Engine {
		struct Options {
			uint32_t fps = 60;
			float pollPeriod = 3.0f;
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

			VkRenderPass renderPass; // TODO: created by window, not engine
			VkCommandPool commandPool;

			VkQueue graphicsQueue;
			VkQueue presentQueue;
			VkQueue transferQueue;

			std::vector<VkCommandBuffer> commandBuffers;

			std::vector<VkSemaphore> imageAvailableSemaphores;
			std::vector<VkSemaphore> renderFinishedSemaphores;
			std::vector<VkFence> inFlightFences;

			uint32_t currentFrameIndex;
			uint32_t currentImageIndex;

			float targetTimePerFrame;
			float pollPeriod;
			float pollFramesElapsedTime;
			float pollUpdatesElapsedTime;
			uint32_t pollFramesCounted;

			Time::Timer frameTimer;
			Time::Timer updateTimer;

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

			void setOptions(const Options& options);

			void pickPhysicalDevice(const std::vector<const char*>& extensions, Window::Handle window);
			void createLogicalDevice(Window::Handle window, const std::span<const char* const> extensions, const std::span<const char* const> validationLayers);

			void createCommandPool(Window::Handle window);
			void createCommandBuffers();

			void createSyncObjects();

			std::vector<const char*> createInstance(const std::span<const char* const> validationLayers, const std::span<const char* const> defaultExtensions);
			void setupDebugMessenger();

			uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
			
			bool isNull() const;

			void checkPerformance();
			void limitFramerate();

			// Public methods
			void create(Options options, Window::Handle window);
			void close(Window::Handle window);

			void beginFrame(Window::Handle window);
			void endFrame(Window::Handle window);
			
			void beginRender(Window::Handle window);
			void endRender();

			Resource();
		};

		typedef Resource* Handle;

		template <Allocator::AllocatorType Storage>
		Handle create(Storage storage, Options options, Window::Handle window)
		{
			Handle engine = storage.allocate(sizeof(Resource));

			new (engine) Resource{};
			
			engine->create(options, window);

			return engine;
		}

		template <Allocator::AllocatorType Storage>
		void close(Storage storage, Handle handle, Window::Handle window)
		{
			handle->close(window);
			storage.free(reinterpret_cast<void*>(handle));
		}

		void beginFrame(Engine::Handle engine, Window::Handle window);
		void endFrame(Engine::Handle engine, Window::Handle window);

		void beginRender(Engine::Handle engine, Window::Handle window);
		void endRender(Engine::Handle engine);
	}
}