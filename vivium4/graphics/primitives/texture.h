#pragma once

#include "../../core.h"
#include "../../engine.h"
#include "../gui/font.h"

namespace Vivium {
	namespace Texture {
		enum class Format {
			RGBA = VK_FORMAT_R8G8B8A8_SRGB,
			MONOCHROME = VK_FORMAT_R8_SRGB
		};

		enum class Filter {
			NEAREST = VK_FILTER_NEAREST,
			LINEAR = VK_FILTER_LINEAR
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

			std::vector<uint8_t> data;

			Format imageFormat;
			Filter imageFilter;

			Specification() = default;
			Specification(const std::vector<uint8_t>& data, int width, int height, int channels, Format imageFormat, Filter imageFilter);
		
			static Specification fromImageFile(const char* imageFile, Format imageFormat, Filter imageFilter);
			static Specification fromFont(Font::Font font, Format imageFormat, Filter imageFilter);
		};

		typedef Resource* Handle;
		typedef Resource* PromisedHandle;

		template <Allocator::AllocatorType AllocatorType>
		void drop(AllocatorType allocator, Handle handle, Engine::Handle engine) {
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine);
			VIVIUM_CHECK_HANDLE_EXISTS(handle);

			handle->drop(engine);

			Allocator::dropResource(allocator, handle);
		}
	}
}