#include "texture.h"

namespace Vivium {
	namespace Texture {
		Image::Image()
			: data(nullptr)
		{}

		Image::Image(Specification specification, void* data)
			: specification(specification), data(data)
		{}

		Image Image::fromFile(const char* filename, Format imageFormat)
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

			uint8_t* data = stbi_load(filename, &specification.width, &specification.height, &specification.channels, stbi_format);

			specification.data = data;
			specification.imageFormat = imageFormat;
			specification.sizeBytes =
				static_cast<uint64_t>(specification.width)
				* static_cast<uint64_t>(specification.height)
				* static_cast<uint64_t>(specification.channels);

			return Image(specification, data);
		}
			
		void Image::drop()
		{
			stbi_image_free(data); 
		}

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
	}
}