#pragma once

#include "../../engine.h"
#include "shader.h"

namespace Vivium {
	enum class BufferUsage : VkFlags {
		STAGING = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VERTEX = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		INDEX = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		UNIFORM = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		STORAGE = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
	};

	// TODO: still feels a little off
	struct BufferLayout {
		struct Element {
			uint32_t size, offset;

			Element() = default;
			Element(uint32_t size, uint32_t offset);
		};

		std::vector<Element> elements;
		uint32_t stride;

		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		VkVertexInputBindingDescription bindingDescription;

		// TODO: shouldn't have static functions
		static BufferLayout fromTypes(const std::span<const ShaderDataType> types);
	};

	struct Buffer {
		VkBuffer buffer;
		void* mapping;
	};

	// TODO: implementation undecided
	struct BufferSlice {
		Buffer buffer;
		uint64_t bufferOffset;
		uint64_t bufferSize;
	};

	struct BufferSpecification {
		uint64_t size;
		BufferUsage usage;
	};

	struct BufferReference {
		uint64_t referenceIndex;
		uint8_t memoryIndex;
	};

	bool isBufferNull(Buffer const& buffer);
	
	void setBuffer(Buffer& buffer, uint64_t bufferOffset, const void* data, uint64_t size);
	void* getBufferMapping(Buffer& buffer);
	
	template <Storage::StorageType StorageType>
	void dropBuffer(StorageType* allocator, Buffer& buffer, Engine::Handle engine)
	{
		vkDestroyBuffer(engine->device, buffer.buffer, nullptr);

		Storage::dropResource(allocator, &buffer);
	}
}