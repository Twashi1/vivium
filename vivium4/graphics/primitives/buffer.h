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
		};

		struct Specification {
			uint64_t size;
			Usage usage;

			Specification() = default;
			Specification(uint64_t size, Usage usage);
		};

		// TODO: by convention, this should have a constructor?
		//		but maybe convention is wrong..
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

			static Layout fromTypes(const std::span<const Shader::DataType> types);
		};
			
		typedef Resource* Handle;
		typedef Resource* PromisedHandle;

		bool isNull(const Handle buffer);
		void set(Handle buffer, uint64_t bufferOffset, const void* data, uint64_t size, uint64_t dataOffset);
		void* getMapping(Handle buffer);

		template <Allocator::AllocatorType AllocatorType>
		void drop(AllocatorType* allocator, Handle buffer, Engine::Handle engine) {
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);
			VIVIUM_CHECK_HANDLE_EXISTS(buffer);

			vkDestroyBuffer(engine->device, buffer->buffer, nullptr);

			VIVIUM_DEBUG_ONLY(buffer = VK_NULL_HANDLE);

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
			};

			struct Specification {
				std::vector<uint32_t> suballocations;
				Usage usage;
			};

			typedef Resource* Handle;
			typedef Resource* PromisedHandle;

			bool isNull(const Handle buffer);
			// Inclusive
			void set(Handle buffer, const void* data, uint64_t suballocationStartIndex, uint64_t suballocationEndIndex);
			void* getMapping(Handle buffer);

			template <Allocator::AllocatorType AllocatorType>
			void drop(AllocatorType* allocator, Handle buffer, Engine::Handle engine) {
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);
				VIVIUM_CHECK_HANDLE_EXISTS(buffer);

				vkDestroyBuffer(engine->device, buffer->buffer, nullptr);

				VIVIUM_DEBUG_ONLY(buffer = VK_NULL_HANDLE);

				Allocator::dropResource(allocator, buffer);
			}
		}
	}
}