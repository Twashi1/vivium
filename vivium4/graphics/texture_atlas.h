#pragma once

#include "../core.h"
#include "../math/vec2.h"

namespace Vivium {
	struct TextureAtlas {
		I32x2 atlasDimensionsSprites;
		I32x2 atlasDimensionsPixels;
		I32x2 spriteDimensions;

		struct Index {
			float left, right, top, bottom;
			// TODO: move to different form of index?
			//	data overhead is negligible
			// Alternate representation
			F32x2 translation;
			F32x2 scale;
			
			void calculate(int left, int right, int bottom, int top, TextureAtlas atlas);

			Index() = default;
			Index(int index, TextureAtlas atlas);
			Index(I32x2 index, TextureAtlas atlas);
			// Inclusive
			Index(I32x2 topLeft, I32x2 bottomRight, TextureAtlas atlas);
		};

		TextureAtlas() = default;
		TextureAtlas(I32x2 atlasDimensionsPixels, I32x2 spriteDimensions);
	};
}