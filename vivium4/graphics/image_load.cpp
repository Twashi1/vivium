#include "image_load.h"

namespace Vivium {
	Image loadImage(char const* filename, TextureFormat format) {
		Image image;

		int stbi_format;

		switch (format) {
		case TextureFormat::RGBA:
			stbi_format = STBI_rgb_alpha; break;
		case TextureFormat::MONOCHROME:
			stbi_format = STBI_grey; break;
		default:
			stbi_format = STBI_default;

			VIVIUM_LOG(LogSeverity::FATAL, "Invalid image format");

			break;
		}

		int channels;

		uint8_t* data = stbi_load(filename, &image.size.x, &image.size.y, &channels, stbi_format);
		VIVIUM_ASSERT(data != nullptr, "Failed to load image file");
		uint64_t imageSize = static_cast<uint64_t>(image.size.x)
			* static_cast<uint64_t>(image.size.y)
			* static_cast<uint64_t>(channels);

		// Copy image data into specification
		image.data = (uint8_t*)std::malloc(imageSize);
		VIVIUM_ASSERT(image.data != nullptr, "Failed to allocate space for image");
		std::memcpy(image.data, data, imageSize);

		image.format = format;

		stbi_image_free(data);

		return image;
	}

	void dropImage(Image& image) {
		std::free(image.data);
		VIVIUM_DEBUG_ONLY(image.data = nullptr);
	}
}