#pragma once

// TODO: move out into graphics folder

#include "../../core.h"
#include "../../engine.h"

namespace Vivium {
	// TODO: want to integrate host visible, coherent, and device local memory
	//		however some systems might not have this available, so needs to account for that
	// TODO: memory type should not need to be restricted to buffers,
	//		should be in Vivium namespace
	// TODO: eventually want to make DYNAMIC_UNIFORM not coherent
	enum class MemoryType {
		STAGING			= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		DEVICE			= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		UNIFORM			= STAGING,
		DYNAMIC_UNIFORM = STAGING
	};

	bool isMappable(MemoryType type);
	uint32_t findMemoryType(Engine& engine, uint32_t typeFilter, VkMemoryPropertyFlags memoryProperties);
}