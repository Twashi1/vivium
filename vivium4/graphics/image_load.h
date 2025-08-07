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

	/*! \brief Load image from a file, given a texture format.
	* 
	* The loaded image will need to be dropped. Uses stbi, many image formats supported.
	* 
	* \param filename The file to load the image from.
	* \param format The texture format of the image.
	* 
	* \return An image object.
	*/
	Image loadImage(char const* filename, TextureFormat format);
	/*! \brief Free the image data on an image.
	* Any copies of the image will also be freed by this.
	* 
	* \param image The image to free.
	*/
	void dropImage(Image& image);
}