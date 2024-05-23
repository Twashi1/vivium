#include "buffer.h"
#include "../resource_manager.h"

namespace Vivium {
	namespace Buffer {
		bool isNull(const Handle buffer)
		{
			return buffer->buffer == VK_NULL_HANDLE;
		}

		void set(Handle buffer, uint64_t bufferOffset, const void* data, uint64_t size, uint64_t dataOffset)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(buffer, Buffer::isNull);

			VIVIUM_ASSERT(size + dataOffset + bufferOffset <= buffer->size,
				"Setting memory OOBs");

			std::memcpy(
				reinterpret_cast<uint8_t*>(buffer->mapping) + bufferOffset,
				reinterpret_cast<const uint8_t*>(data) + dataOffset,
				size
			);
		}

		void* getMapping(Handle buffer)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(buffer, Buffer::isNull);

			return buffer->mapping;
		}

		void drop(ResourceManager::Static::Handle manager, Handle buffer, Engine::Handle engine)
		{
			VIVIUM_CHECK_HANDLE_EXISTS(manager);
			VIVIUM_CHECK_HANDLE_EXISTS(buffer);

			manager->drop(buffer, engine);
		}
			
		Specification::Specification(uint64_t size, Usage usage)
			: size(size), usage(usage)
		{}
			
		Layout::Element::Element(uint32_t size, uint32_t offset)
			: size(size), offset(offset)
		{}

		namespace Dynamic {
			bool isNull(const Handle buffer)
			{
				return buffer->buffer == VK_NULL_HANDLE;
			}

			// Inclusive
			void set(Handle buffer, const void* data, uint64_t suballocationStartIndex, uint64_t suballocationEndIndex) {
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(buffer, Buffer::Dynamic::isNull);

				// Advance into data passed, just advance by size of each sub-allocation
				uint64_t totalSourceAdvance = 0;

				for (uint64_t i = suballocationStartIndex; i <= suballocationEndIndex; i++) {
					VIVIUM_ASSERT(i < buffer->suballocationOffsets.size(), "Invalid suballocation end index, went OOBs");

					std::memcpy(
						reinterpret_cast<uint8_t*>(buffer->mapping)
						+ buffer->suballocationOffsets[i],
						reinterpret_cast<const uint8_t*>(data)
						+ totalSourceAdvance,
						buffer->suballocationSizes[i]
					);

					totalSourceAdvance += buffer->suballocationSizes[i];
				}
			}
			
			void* getMapping(Handle buffer)
			{
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(buffer, Buffer::Dynamic::isNull);

				return buffer->mapping;
			}
		}
		
		Layout Layout::fromTypes(const std::span<const Shader::DataType> types)
		{
			Layout layout;

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
				layout.elements[index] = Layout::Element(size_of_type, currentOffset);

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
}