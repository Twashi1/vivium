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
			uint32_t graphicsFamily, presentFamily, transferFamily;

			bool isComplete() const { return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX && transferFamily != UINT32_MAX; }
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

	// TODO: consider moving to window?
	Engine::QueueFamilyIndices _findQueueFamilies(Engine& engine, VkPhysicalDevice device);

	bool _checkValidationLayerSupport(const std::span<const char* const>& validationLayers);
	bool _isSuitableDevice(Engine& engine, VkPhysicalDevice device, const std::vector<const char*>& requiredExtensions, Window& window);
	bool _checkDeviceExtensionSupport(const std::vector<const char*>& requiredExtensions, VkPhysicalDevice device);

	// TODO: consider moving to window
	Engine::SwapChainSupportDetails _querySwapChainSupport(VkPhysicalDevice device, Window& window);

	void _setOptions(Engine& engine, EngineOptions const& options);

	void _pickPhysicalDevice(Engine& engine, const std::vector<const char*>& extensions);
	void _createLogicalDevice(Engine& engine, const std::span<const char* const> extensions, const std::span<const char* const> validationLayers);

	void _createInstance(Engine& engine, const std::span<const char* const> validationLayers, const std::span<const char* const> defaultExtensions);
	void _setupDebugMessenger(Engine& engine);

	// TODO: isn't this somewhere else?
	uint32_t _findMemoryType(Engine& engine, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void _checkPerformance(Engine& engine);
	void _limitFramerate(Engine& engine);

	// Public methods
	Engine createEngine(EngineOptions const& options);
	void dropEngine(Engine& engine);

	void engineBeginFrame(Engine& engine, CommandContext& context);
	void engineEndFrame(Engine& engine);
}