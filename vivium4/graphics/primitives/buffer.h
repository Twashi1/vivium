#pragma once

#include <cstdint>

#include "../../core.h"

namespace Vivium {
	namespace Graphics {
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
				void close();
				void set(const void* data, uint64_t size, uint64_t offset);
			};

			struct Specification {
				uint64_t size;
				Usage usage;

				Specification() = default;
				Specification(uint64_t size, Usage usage);
			};

			struct Layout {

			};

			typedef Resource* Handle;
			typedef Resource* DynamicHandle;

			void set(Handle handle, const void* data, uint64_t size, uint64_t offset);
			void close(Handle handle);
			void close(DynamicHandle handle);
		}
	}
}