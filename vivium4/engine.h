#pragma once

#include <array>
#include <set>
#include <span>

#include "storage.h"
#include "core.h"
#include "error/log.h"
#include "window.h"

namespace Vivium {
	struct EngineOptions {
		uint32_t fps = 60;
		// Performance poll period
		float pollPeriod = 3.0f;
	};

	struct CommandContext;

	struct Engine {
		// TODO: move out of class?
		static constexpr bool enableValidationLayers = VIVIUM_IS_DEBUG;

		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		struct QueueFamilyIndices {
			uint32_t graphicsFamily;
			uint32_t presentFamily;
			uint32_t transferFamily;
		};

		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;

		VkPhysicalDevice physicalDevice;
		VkDevice device;

		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkQueue transferQueue;

		float targetTimePerFrame;
		float pollPeriod;
		float pollFramesElapsedTime;
		float pollUpdatesElapsedTime;
		uint32_t pollFramesCounted;

		Time::Timer frameTimer;
		Time::Timer updateTimer;
	};

	void _populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	static VKAPI_ATTR VkBool32 VKAPI_CALL _vulkanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT vkSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
		void* userData
	);

	Engine::QueueFamilyIndices _findQueueFamilies(VkPhysicalDevice device);

	bool _checkValidationLayerSupport(const std::span<const char* const>& validationLayers);
	bool _checkSurfaceSupport(Engine& engine, VkSurfaceKHR surface);
	bool _checkDeviceExtensionSupport(const std::vector<const char*>& requiredExtensions, VkPhysicalDevice device);

	Engine::SwapChainSupportDetails _querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

	void _setOptions(Engine& engine, EngineOptions const& options);

	void _pickPhysicalDevice(Engine& engine, const std::vector<const char*>& extensions);
	void _createLogicalDevice(Engine& engine, const std::span<const char* const> extensions, const std::span<const char* const> validationLayers);

	void _createInstance(Engine& engine, const std::span<const char* const> validationLayers, const std::span<const char* const> defaultExtensions);
	void _setupDebugMessenger(Engine& engine);

	void _checkPerformance(Engine& engine);
	void _limitFramerate(Engine& engine);

	// Public methods
	Engine createEngine(EngineOptions const& options);
	void dropEngine(Engine& engine);

	void engineBeginFrame(Engine& engine, CommandContext& context);
	void engineEndFrame(Engine& engine);
}