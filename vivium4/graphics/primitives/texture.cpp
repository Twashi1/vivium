#include "texture.h"
#include "../resource_manager.h"

namespace Vivium {
	TextureSpecification TextureSpecification::fromImageFile(const char* imageFile, TextureFormat imageFormat, TextureFilter imageFilter)
	{
		TextureSpecification specification;

		int stbi_format;

		switch (imageFormat) {
		case TextureFormat::RGBA:
			stbi_format = STBI_rgb_alpha; break;
		case TextureFormat::MONOCHROME:
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
		specification.imageFilter = imageFilter;

		stbi_image_free(data);

		return specification;
	}
		
	TextureSpecification TextureSpecification::fromFont(Font::Font font, TextureFormat imageFormat, TextureFilter imageFilter)
	{
		Specification specification;

		// TODO; function for getting channels/stbi_image_format
		int stbi_format;

		switch (imageFormat) {
		case TextureFormat::RGBA:
			stbi_format = STBI_rgb_alpha; break;
		case TextureFormat::MONOCHROME:
			stbi_format = STBI_grey; break;
		default:
			VIVIUM_LOG(Log::FATAL, "Invalid image format");

			break;
		}

		specification.data = font.data;
		specification.width = font.imageDimensions.x;
		specification.height = font.imageDimensions.y;
		specification.channels = stbi_format;
		specification.imageFilter = imageFilter;
		specification.imageFormat = imageFormat;

		return specification;
	}
}