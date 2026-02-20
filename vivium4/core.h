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

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include <cstring>
#include <format>
#include <iostream>
#include <string>

#include "error/log.h"
#include "time/timer.h"
#include "vulkan/vk_enum_string_helper.h"

#ifdef NDEBUG
#define VIVIUM_DEBUG_ONLY(statement) ((void)0)
#define VIVIUM_ASSERT(condition, msg, ...) ((void)0)
#define VIVIUM_VK_CHECK(command, message) command
#define VIVIUM_IS_DEBUG 0
#else
#define VIVIUM_DEBUG_ONLY(statement) statement
#define VIVIUM_ASSERT(condition, msg, ...)                                \
  if (!(condition))                                                       \
  VIVIUM_LOG(Vivium::LogSeverity::FATAL, "[ASSERT] ({}): {}", #condition, \
             std::format(msg __VA_OPT__(, ) __VA_ARGS__))
#define VIVIUM_VK_CHECK(command, message)                  \
  if (VkResult result = command; result != VK_SUCCESS)     \
  VIVIUM_LOG(Vivium::LogSeverity::ERROR, "[VULKAN:{}] {}", \
             string_VkResult(result), message)
#define VIVIUM_IS_DEBUG 1
#endif

// TODO: weird place for this variable
#define VIVIUM_FRAMES_IN_FLIGHT 2

#if defined(_WIN32)
#define VIVIUM_PLATFORM_WINDOWS
#elif defined(__linux__)
#define VIVIUM_PLATFORM_LINUX
#else
static_assert("Unknown platform")
#endif

#if defined(VIVIUM_PLATFORM_WINDOWS)
#define VIVIUM_GLSLC_PATH "external/win/vulkan/Bin/glslc.exe"
#elif defined(VIVIUM_PLATFORM_LINUX)
#define VIVIUM_GLSLC_PATH "external/vulkansdk/glslc"
#endif
