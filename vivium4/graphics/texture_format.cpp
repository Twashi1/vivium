#include "texture_format.h"

namespace Vivium {
	// TODO: log warnings
	int getTextureFormatStride(TextureFormat format)
	{
		switch (format) {
		case TextureFormat::RGBA: return 4;
		case TextureFormat::MONOCHROME: return 1;
		default: return -1;
		}
	}
	
	int getTextureFormatChannels(TextureFormat format)
	{
		switch (format) {
		case TextureFormat::RGBA: return 4;
		case TextureFormat::MONOCHROME: return 1;
		default: return -1;
		}
	}
}