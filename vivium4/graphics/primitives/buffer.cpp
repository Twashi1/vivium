#include "buffer.h"

namespace Vivium {
	namespace Buffer {
		bool Resource::isNull() const
		{
			return buffer == nullptr;
		}
			
		void Resource::drop(Engine::Handle engine)
		{
			vkDestroyBuffer(engine->device, buffer, nullptr);

			VIVIUM_DEBUG_ONLY(buffer = VK_NULL_HANDLE);
		}
			
		void Resource::set(const void* data, uint64_t size, uint64_t offset)
		{
			std::memcpy(
				reinterpret_cast<uint8_t*>(mapping) + offset,
				data,
				size
			);
		}
			
		Layout createLayout(const std::span<Shader::DataType> types)
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

		void set(Handle buffer, const void* data, uint64_t size, uint64_t offset)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(buffer);

			VIVIUM_ASSERT(size + offset <= buffer->size,
				"Setting memory OOBs");

			buffer->set(data, size, offset);
		}
			
		Specification::Specification(uint64_t size, Usage usage)
			: size(size), usage(usage)
		{}
			
		Layout::Element::Element(uint32_t size, uint32_t offset)
			: size(size), offset(offset)
		{}

		namespace Dynamic {
			bool Resource::isNull() const
			{
				return buffer == VK_NULL_HANDLE;
			}

			void Resource::drop(Engine::Handle engine)
			{
				vkDestroyBuffer(engine->device, buffer, nullptr);

				VIVIUM_DEBUG_ONLY(buffer = VK_NULL_HANDLE);
			}

			void Resource::set(const void* data, uint64_t suballocationStartIndex, uint64_t suballocationEndIndex)
			{
				// Advance into data passed, just advance by size of each sub-allocation
				uint64_t totalSourceAdvance = 0;

				for (uint64_t i = suballocationStartIndex; i <= suballocationEndIndex; i++) {
					VIVIUM_ASSERT(i < suballocationOffsets.size(), "Invalid suballocation end index, went OOBs");

					std::memcpy(
						reinterpret_cast<uint8_t*>(mapping)
							+ suballocationOffsets[i],
						reinterpret_cast<const uint8_t*>(data)
							+ totalSourceAdvance,
						suballocationSizes[i]
					);

					totalSourceAdvance += suballocationSizes[i];
				}
			}
		}
	}
}