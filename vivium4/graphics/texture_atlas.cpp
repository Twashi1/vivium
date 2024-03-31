#include "texture_atlas.h"

namespace Vivium {
	void TextureAtlas::Index::calculate(int left, int right, int bottom, int top, TextureAtlas atlas)
	{
		float inv_width = 1.0f / atlas.atlasDimensionsPixels.x;
		float inv_height = 1.0f / atlas.atlasDimensionsPixels.y;

		// Calculate texture coordinates
		this->left = left * inv_width * atlas.spriteDimensions.x;
		this->right = right * inv_width * atlas.spriteDimensions.x;
		// Vertical flip here
		this->bottom = top * inv_height * atlas.spriteDimensions.y;
		this->top = bottom * inv_height * atlas.spriteDimensions.y;

		translation = F32x2(this->left, this->top);
		scale = F32x2(this->right - this->left, this->bottom - this->top);
	}


	TextureAtlas::Index::Index(int index, TextureAtlas atlas)
	{
		int yIndex = index / atlas.atlasDimensionsSprites.x;
		int xIndex = index - yIndex * atlas.atlasDimensionsSprites.x;

		calculate(xIndex, xIndex + 1, yIndex, yIndex + 1, atlas);
	}

	TextureAtlas::Index::Index(I32x2 index, TextureAtlas atlas)
	{
		calculate(index.x, index.x + 1, index.y, index.y + 1, atlas);
	}

	TextureAtlas::Index::Index(I32x2 topLeft, I32x2 bottomRight, TextureAtlas atlas)
	{
		calculate(topLeft.x, bottomRight.x + 1, bottomRight.y, topLeft.y + 1, atlas);
	}
	
	TextureAtlas::TextureAtlas(I32x2 atlasDimensionsPixels, I32x2 spriteDimensions)
		: atlasDimensionsPixels(atlasDimensionsPixels), spriteDimensions(spriteDimensions), atlasDimensionsSprites(atlasDimensionsPixels / spriteDimensions)
	{
		VIVIUM_ASSERT(atlasDimensionsSprites * spriteDimensions == atlasDimensionsPixels, "Sprite dimension must be factor of atlas dimension");
	}
}