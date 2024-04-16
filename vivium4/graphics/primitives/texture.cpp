#include "texture.h"

namespace Vivium {
	namespace Texture {
		bool Resource::isNull() const
		{
			return image == VK_NULL_HANDLE;
		}

		void Resource::drop(Engine::Handle engine)
		{
			vkDestroySampler(engine->device, sampler, nullptr);
			vkDestroyImageView(engine->device, view, nullptr);
			vkDestroyImage(engine->device, image, nullptr);
		}
		
		Specification::Specification(const std::vector<uint8_t>& data, int width, int height, int channels, Format imageFormat, Filter imageFilter)
			: data(data), width(width), height(height), channels(channels), imageFormat(imageFormat), imageFilter(imageFilter)
		{}
		
		Specification Specification::fromImageFile(const char* imageFile, Format imageFormat, Filter imageFilter)
		{
			Specification specification;

			int stbi_format;

			switch (imageFormat) {
			case Format::RGBA:
				stbi_format = STBI_rgb_alpha; break;
			case Format::MONOCHROME:
				stbi_format = STBI_grey; break;
			default:
				stbi_format = STBI_default;

				VIVIUM_LOG(Log::FATAL, "Invalid image format");

				break;
			}

			uint8_t* data = stbi_load(imageFile, &specification.width, &specification.height, &specification.channels, stbi_format);
			uint64_t imageSize = static_cast<uint64_t>(specification.width)
				* static_cast<uint64_t>(specification.height)
				* static_cast<uint64_t>(specification.channels);

			// Copy image data into specification
			specification.data = std::vector<uint8_t>(imageSize);
			std::memcpy(specification.data.data(), data, imageSize);

			specification.imageFormat = imageFormat;

			stbi_image_free(data);

			return specification;
		}
		
		Specification Specification::fromFont(Font::Font font, Format imageFormat, Filter imageFilter)
		{
			Specification specification;

			specification.data = font.data;

			return specification;
		}
	}
}