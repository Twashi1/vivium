#pragma once

#include <array>
#include <cstdint>
#include <fstream>
#include <span>

#include <ft2build.h>
#include <freetype/freetype.h>

#include "../texture_atlas.h"
#include "../../storage.h"

#define VIVIUM_CHARACTERS_TO_EXTRACT 128

namespace Vivium {
	namespace Font {
		inline FT_Library ftLibrary;

		void init();
		void terminate();

		struct Character {
			I32x2 size, bearing;
			int advance;

			float left, right, bottom, top;
		};

		struct Font {
			std::vector<uint8_t> data;

			I32x2 imageDimensions;
			int fontSize;

			TextureAtlas atlas;

			std::array<Character, VIVIUM_CHARACTERS_TO_EXTRACT> characters;

			static Font fromFile(const char* filename, int fontSize);
			static Font fromDistanceFieldFile(const char* filename);
		};
		
		// https://cdn.akamai.steamstatic.com/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf
		// https://libgdx.com/wiki/graphics/2d/fonts/distance-field-fonts
		void _computeSignedDistanceField(const uint8_t* input, uint64_t inputWidth, uint64_t inputHeight, uint8_t* output, uint64_t outputWidth, uint64_t outputHeight, float spreadFactor);
		// TODO: actually compile to file directly (while still returning the Font)
		Font compileSignedDistanceField(const char* inputFontFile, int inputFontSize, const char* outputFile, int outputFieldsize, float spreadFactor);
	}
}