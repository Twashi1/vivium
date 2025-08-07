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

	/*! \brief Create an instance of the engine.
	* \param options Options to set framerate and poll frequency.
	* \return An instance of the engine.
	*/
	Engine createEngine(EngineOptions const& options);
	/*! \brief Free the engine once done.
	* 
	* Must free all other resources beforehand.
	* 
	* \param engine The engine instance to free.
	*/
	void dropEngine(Engine& engine);

	/*! \brief Perform processing to begin a frame.
	* 
	* All subsequent relevant actions will be attached to the new frame.
	* 
	* \param engine The engine to begin the frame on.
	* \param context The command context for which all frame actions will be performed on.
	*/
	void engineBeginFrame(Engine& engine, CommandContext& context);
	/*! \brief Perform processing to end a frame.
	* 
	* Can stall to meet framerate limit.
	* 
	* \param engine The engine to end the frame on.
	*/
	void engineEndFrame(Engine& engine);
}