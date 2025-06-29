#pragma once

#include <array>
#include <cstdint>
#include <fstream>
#include <span>

#include <ft2build.h>
#include <freetype/freetype.h>

#include "../../storage.h"
#include "../../math/atlas.h"

#define VIVIUM_CHARACTERS_TO_EXTRACT 128

namespace Vivium {
	inline FT_Library ftLibrary;

	void _fontInit();
	void _fontTerminate();

	struct FontCharacter {
		I32x2 size, bearing;
		int advance;

		float left, right, bottom, top;
	};

	struct Font {
		std::vector<uint8_t> data;

		I32x2 imageDimensions;
		int fontSize;
		I32x2 fontSpriteSize;

		std::array<FontCharacter, VIVIUM_CHARACTERS_TO_EXTRACT> characters;
	};

	Font createFontFile(const char* filename, int fontSize);
	Font createFontDistanceField(const char* filename);

	// https://cdn.akamai.steamstatic.com/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf
	// https://libgdx.com/wiki/graphics/2d/fonts/distance-field-fonts
	// TODO: inconsistent style with pre-pend underscore
	void _computeSignedDistanceField(const uint8_t* input, uint64_t inputWidth, uint64_t inputHeight, uint8_t* output, uint64_t outputWidth, uint64_t outputHeight, float spreadFactor);
	Font compileSignedDistanceField(const char* inputFontFile, int inputFontSize, const char* outputFile, int outputFieldsize, float spreadFactor);
	
	void writeDistanceFieldFont(const char* outputFontFile, Font font);
}