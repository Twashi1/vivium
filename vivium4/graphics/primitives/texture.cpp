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

			VIVIUM_LOG(LogSeverity::FATAL, "Invalid image format");

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

		// TODO: realistically should just call TextureSpecification::fromData (define recursively where possible)

		stbi_image_free(data);

		return specification;
	}
		
	TextureSpecification TextureSpecification::fromFont(Font font, TextureFormat imageFormat, TextureFilter imageFilter)
	{
		TextureSpecification specification;

		// TODO; function for getting channels/stbi_image_format
		int stbi_format;

		switch (imageFormat) {
		case TextureFormat::RGBA:
			stbi_format = STBI_rgb_alpha; break;
		case TextureFormat::MONOCHROME:
			stbi_format = STBI_grey; break;
		default:
			VIVIUM_LOG(LogSeverity::FATAL, "Invalid image format");

			break;
		}

		// TODO: realistically should just call TextureSpecification::fromData (define recursively where possible)

		specification.data = font.data;
		specification.width = font.imageDimensions.x;
		specification.height = font.imageDimensions.y;
		specification.channels = stbi_format;
		specification.imageFilter = imageFilter;
		specification.imageFormat = imageFormat;

		return specification;
	}

	TextureSpecification TextureSpecification::fromData(uint8_t const* data, I32x2 dimensions, TextureFormat imageFormat, TextureFilter imageFilter)
	{
		TextureSpecification specification;

		specification.data.resize(dimensions.x * dimensions.y * getTextureFormatStride(imageFormat));
		memcpy(specification.data.data(), data, specification.data.size());

		specification.width = dimensions.x;
		specification.height = dimensions.y;
		specification.channels = getTextureFormatChannels(imageFormat);
		specification.imageFilter = imageFilter;
		specification.imageFormat = imageFormat;

		return specification;
	}

	TextureSpecification TextureSpecification::fromImage(Image image, TextureFilter imageFilter)
	{
		return TextureSpecification::fromData(image.data, image.size, image.format, imageFilter);
	}

	void dropTexture(Texture& texture, Engine& engine) {
		vkDestroySampler(engine.device, texture.sampler, nullptr);
		vkDestroyImageView(engine.device, texture.view, nullptr);
		vkDestroyImage(engine.device, texture.image, nullptr);
	}
}