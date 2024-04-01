#pragma once

#include "../../core.h"
#include "../../engine.h"

namespace Vivium {
	namespace Texture {
		enum class Format {
			RGBA = VK_FORMAT_R8G8B8A8_SRGB,
			MONOCHROME = VK_FORMAT_R8_SRGB
		};

		struct Resource {
			VkImage image;
			VkImageView view;
			VkSampler sampler;

			bool isNull() const;
			void drop(Engine::Handle engine);
		};

		struct Specification {
			int width, height, channels;

			const uint8_t* data;
			uint64_t sizeBytes;

			Format imageFormat;
		};

		struct Image {
			Specification specification;
			void* data;

			Image();
			Image(Specification specification, void* data);

			static Image fromFile(const char* filename, Format imageFormat);

			void drop();
		};

		typedef Resource* Handle;

		template <Allocator::AllocatorType AllocatorType>
		void drop(AllocatorType allocator, Handle handle, Engine::Handle engine) {
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine);
			VIVIUM_CHECK_HANDLE_EXISTS(handle);

			handle->drop(engine);

			Allocator::dropResource(allocator, handle);
		}
	}
}