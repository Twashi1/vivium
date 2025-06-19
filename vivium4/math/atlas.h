#pragma once

#include "vec2.h"
#include "../core.h"
#include "../graphics/texture_format.h"
#include "../graphics/image_load.h"

// TODO: move out of math path?
// Support for stitching multiple atlases together
//	when stitching multiple atlases together, it should return the atlas index for each stitched atlas
//	most likely its possible to return a reference such that when the final atlas is stiched
//	we can retrieve all relevant information (left/right/top/bottom) without having to store additional
//	metadata

// https://www.david-colson.com/2020/03/10/exploring-rect-packing.html
// Note we don't even have full information beforehand, we have to greedily pack them
//	so we're never going to get a good algorithm
//	just try to fit by row?

namespace Vivium {
	struct AtlasIndex {
		// Texture coordinate format
		float left;
		float right;
		float top;
		float bottom;

		// Alternative format (For instance rendering)
		F32x2 translation;
		F32x2 scale;
	};

	struct StitchedAtlasReference {
		I32x2 size;
		I32x2 offset;
	};

	struct StitchedAtlasAllocation {
		StitchedAtlasReference reference;
		uint8_t* data;
	};

	struct StitchedAtlasCreator {
		std::vector<StitchedAtlasAllocation> allocations;
		TextureFormat format;
		uint64_t xOffset; // Relatively naive approach to packing, just put it all in a row
	};

	// TODO: textures loading upside-down
	struct StitchedAtlas {
		uint8_t* data;
		I32x2 size;
		TextureFormat format;
	};

	StitchedAtlasCreator createStitchedAtlasCreator(TextureFormat format);
	
	StitchedAtlasReference submitToStitchedAtlasCreator(const char* imageFilename, StitchedAtlasCreator& atlasCreator);
	StitchedAtlasReference submitToStitchedAtlasCreator(I32x2 size, uint8_t* data, StitchedAtlasCreator& atlasCreator);
	StitchedAtlas finishAtlasCreation(StitchedAtlasCreator const& atlasCreator);
	void dropAtlasCreator(StitchedAtlasCreator& atlasCreator);
	AtlasIndex convertStitchedAtlasReference(StitchedAtlasReference reference, StitchedAtlas const& atlas);
	void dropAtlas(StitchedAtlas& atlas);

	// Assumes rectangle atlas with entries organised as a grid
	AtlasIndex _calculateAtlasIndex(int left, int right, int bottom, int top, I32x2 atlasSize, I32x2 spriteSize);
	AtlasIndex textureAtlasIndex(I32x2 atlasSize, I32x2 spriteSize, int index);
	AtlasIndex textureAtlasIndex(I32x2 atlasSize, I32x2 spriteSize, I32x2 index);
	AtlasIndex textureAtlasIndex(I32x2 atlasSize, I32x2 spriteSize, I32x2 topLeft, I32x2 bottomRight);
}