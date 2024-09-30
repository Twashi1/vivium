#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "stb_image.h"
#include "stb_image_write.h"

#include <format>
#include <iostream>
#include <string>

#include "time/timer.h"
#include "error/log.h"
#include "vulkan/vk_enum_string_helper.h"

#ifdef NDEBUG
#define VIVIUM_DEBUG_ONLY(statement)							((void)0)
#define VIVIUM_ASSERT(condition, msg, ...)						((void)0)
#define VIVIUM_VK_CHECK(command, message)						command
#define VIVIUM_IS_DEBUG 0
#else
#define VIVIUM_DEBUG_ONLY(statement) statement
#define VIVIUM_ASSERT(condition, msg, ...)			\
	if (!(condition))								\
		VIVIUM_LOG(									\
			Vivium::Log::FATAL,						\
			"[ASSERT] ({}): {}",					\
			#condition,								\
			std::format(msg, __VA_ARGS__)			\
		)
#define VIVIUM_VK_CHECK(command, message) \
	if (VkResult result = command; result != VK_SUCCESS) \
		VIVIUM_LOG(										 \
			Vivium::Log::ERROR,							 \
			"[VULKAN:{}] {}",							 \
			string_VkResult(result),					 \
			message										 \
		)
#define VIVIUM_IS_DEBUG 1
#endif

#define VIVIUM_FRAMES_IN_FLIGHT 2

#define VIVIUM_GLSLC_PATH "external/vulkan/Bin/glslc.exe"