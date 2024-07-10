#pragma once

#include "../../core.h"
#include "../../engine.h"
#include "../gui/font.h"

namespace Vivium {
	enum class TextureFormat {
		RGBA = VK_FORMAT_R8G8B8A8_SRGB,
		MONOCHROME = VK_FORMAT_R8_UNORM
	};

	enum class TextureFilter {
		NEAREST = VK_FILTER_NEAREST,
		LINEAR = VK_FILTER_LINEAR
	};

	struct Texture {
		VkImage image;
		VkImageView view;
		VkSampler sampler;
	};

	struct TextureSpecification {
		int width, height, channels;

		std::vector<uint8_t> data;

		TextureFormat imageFormat;
		TextureFilter imageFilter;

		// TODO: from raw data
		static TextureSpecification fromImageFile(const char* imageFile, TextureFormat imageFormat, TextureFilter imageFilter);
		static TextureSpecification fromFont(Font::Font font, TextureFormat imageFormat, TextureFilter imageFilter);
	};
	
	struct TextureReference {
		uint64_t referenceIndex;
	};

	void dropTexture(Texture& texture, Engine::Handle engine);
}