#include "buffer.h"

namespace Vivium {
	bool isBufferNull(Buffer const& buffer)
	{
		return buffer.buffer == VK_NULL_HANDLE;
	}

	void setBuffer(Buffer const& buffer, uint64_t bufferOffset, const void* data, uint64_t size)
	{
		VIVIUM_CHECK_RESOURCE_EXISTS(buffer, Vivium::isBufferNull);

		// TODO: make assertion work in debug mode
#ifdef 0
		VIVIUM_ASSERT(size + bufferOffset <= buffer->size,
			"Setting memory OOBs");
#endif

		std::memcpy(
			reinterpret_cast<uint8_t*>(buffer.mapping) + bufferOffset,
			reinterpret_cast<const uint8_t*>(data),
			size
		);
	}

	void* getBufferMapping(Buffer const& buffer)
	{
		VIVIUM_CHECK_RESOURCE_EXISTS(buffer, Vivium::isBufferNull);

		return buffer.mapping;
	}
			
	BufferLayout::Element::Element(uint32_t size, uint32_t offset)
		: size(size), offset(offset)
	{}
		
	BufferLayout BufferLayout::fromTypes(const std::span<const Shader::DataType> types)
	{
		BufferLayout layout;

		layout.attributeDescriptions.resize(types.size());
		layout.elements.resize(types.size());

		uint32_t currentOffset = 0;

		for (uint32_t index = 0; index < types.size(); index++) {
			Shader::DataType type = types[index];
			uint32_t size_of_type = Shader::sizeOf(type);
			VkFormat format_of_type = Shader::formatOf(type);

			VkVertexInputAttributeDescription attribute{};
			// TODO: customiseable binding
			attribute.binding = 0;
			// Assuming passed in chronological order
			attribute.location = index;
			attribute.format = format_of_type;
			attribute.offset = currentOffset;

			layout.attributeDescriptions[index] = attribute;
			layout.elements[index] = BufferLayout::Element(size_of_type, currentOffset);

			currentOffset += size_of_type;
		}

		// Current offset is now the stride
		layout.bindingDescription.binding = 0;
		// TODO: allow different vertex input methods
		layout.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		layout.bindingDescription.stride = currentOffset;
		layout.stride = currentOffset;

		return layout;
	}
}