#pragma once

#include "../../engine.h"
#include "shader.h"

namespace Vivium {
	namespace Buffer {
		enum class Usage : VkFlags {
			STAGING		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VERTEX		= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			INDEX		= VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			UNIFORM		= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			STORAGE		= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
		};

		struct Resource {
			VkBuffer buffer;
			// Mapping is nullptr for unmappable types
			void* mapping;
			// Size of data stored in the buffer (allocation size will be larger for alignment requirements)
			uint64_t size;
			Usage usage;

			bool isNull() const;
			void drop(Engine::Handle engine);
			void set(const void* data, uint64_t size, uint64_t offset);
		};

		struct Specification {
			uint64_t size;
			Usage usage;

			Specification() = default;
			Specification(uint64_t size, Usage usage);
		};

		struct Layout {
			struct Element {
				uint32_t size, offset;

				Element() = default;
				Element(uint32_t size, uint32_t offset);
			};

			std::vector<Element> elements;
			uint32_t stride;

			std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
			VkVertexInputBindingDescription bindingDescription;
		};
			
		Layout createLayout(const std::span<Shader::DataType> types);

		typedef Resource* Handle;
		// TODO: past thomas how does this work?
		typedef Resource* DynamicHandle;

		void set(Handle buffer, const void* data, uint64_t size, uint64_t offset);

		template <Allocator::AllocatorType AllocatorType>
		void drop(AllocatorType allocator, Engine::Handle engine, Handle buffer) {
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine);
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(buffer);

			buffer->drop(engine);
			Allocator::dropResource(allocator, buffer);
		}

		namespace Dynamic {
			struct Resource {
				// TODO: this is unset right now and unused, since dynamic buffers are coherent
				//	in future, would need to store necessary data to calculate where a the buffer
				//	is mapped, so appropriate range can be flushed
				VkDeviceMemory memory;	// Non-owning handle to memory for flush
				VkBuffer buffer;

				void* mapping;
				Usage usage;

				std::vector<uint32_t> suballocationSizes;
				std::vector<uint32_t> suballocationOffsets;

				bool isNull() const;
				void drop(Engine::Handle engine);

				// Inclusive
				void set(const void* data, uint64_t suballocationStartIndex, uint64_t suballocationEndIndex);
			};

			struct Specification {
				std::vector<uint32_t> suballocations;
				Usage usage;
			};

			typedef Resource* Handle;

			// Inclusive
			void set(Handle buffer, const void* data, uint64_t suballocationStartIndex, uint64_t suballocationEndIndex);

			template <Allocator::AllocatorType AllocatorType>
			void drop(AllocatorType allocator, Engine::Handle engine, Handle buffer) {
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine);
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(buffer);

				buffer->drop(engine);
				Allocator::dropResource(allocator, buffer);
			}
		}
	}
}