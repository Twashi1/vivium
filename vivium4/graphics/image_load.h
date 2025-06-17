#pragma once

#include "texture_format.h"
#include "../math/vec2.h"

// TODO: might be better to have unified texture_format/image_load utility header
//	seems to bare
// TODO: otherwise at least rename to image.h

namespace Vivium {
	struct Image {
		uint8_t* data;
		I32x2 size;

		TextureFormat format;
	};

	Image loadImage(char const* filename, TextureFormat format);
	void dropImage(Image& image);
}