#pragma once

#include "../core.h"

namespace Vivium {
	enum class TextureFormat {
		RGBA = VK_FORMAT_R8G8B8A8_SRGB,
		MONOCHROME = VK_FORMAT_R8_UNORM
	};

	int getTextureFormatStride(TextureFormat format);
	int getTextureFormatChannels(TextureFormat format);
}