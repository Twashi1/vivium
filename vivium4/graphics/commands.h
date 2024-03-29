#pragma once

#include "../engine.h"
#include "primitives/buffer.h"

namespace Vivium {
	namespace Commands {
		void createBuffer(Engine::Handle engine, VkBuffer* buffer, uint64_t size, Buffer::Usage usage, VkMemoryRequirements* memoryRequirements) {
			VkBufferCreateInfo bufferCreateInfo{};
			bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCreateInfo.size = size;
			bufferCreateInfo.usage = static_cast<VkBufferUsageFlags>(usage);
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VIVIUM_VK_CHECK(vkCreateBuffer(engine->device, &bufferCreateInfo, nullptr, buffer), "Failed to create buffer");
			vkGetBufferMemoryRequirements(engine->device, *buffer, memoryRequirements);
		}
	}
}