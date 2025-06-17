#include "atlas.h"

namespace Vivium {
	StitchedAtlasCreator createStitchedAtlasCreator(TextureFormat format)
	{
		StitchedAtlasCreator creator;

		creator.format = format;
		creator.xOffset = 0;

		return creator;
	}

	StitchedAtlasReference submitToStitchedAtlasCreator(const char* imageFilename, StitchedAtlasCreator& atlasCreator)
	{
		// TODO: lots of unneccessary copies/moves
		Image image = loadImage(imageFilename, atlasCreator.format);

		StitchedAtlasReference reference = submitToStitchedAtlasCreator(image.size, image.data, atlasCreator);

		dropImage(image);

		return reference;
	}

	StitchedAtlasReference submitToStitchedAtlasCreator(I32x2 size, uint8_t* data, StitchedAtlasCreator& atlasCreator)
	{
		StitchedAtlasAllocation allocation;

		int stride = getTextureFormatStride(atlasCreator.format);
		
		uint64_t allocationSize = size.x * size.y * stride;

		allocation.data = (uint8_t*)std::malloc(allocationSize);
		VIVIUM_ASSERT(allocation.data != nullptr, "Failed allocation for stitched atlas allocation of size {}", allocationSize);

		std::memcpy(allocation.data, data, allocationSize);

		StitchedAtlasReference reference;
		reference.size = size;
		reference.offset = I32x2(atlasCreator.xOffset, 0);
		atlasCreator.xOffset += size.x;

		allocation.reference = reference;

		atlasCreator.allocations.push_back(allocation);

		return reference;
	}

	StitchedAtlas finishAtlasCreation(StitchedAtlasCreator const& atlasCreator)
	{
		StitchedAtlas atlas;

		int stride = getTextureFormatStride(atlasCreator.format);

		atlas.size = I32x2(0);
		atlas.size.x = atlasCreator.xOffset;
		atlas.format = atlasCreator.format;

		for (uint64_t i = 0; i < atlasCreator.allocations.size(); i++) {
			StitchedAtlasAllocation const& allocation = atlasCreator.allocations[i];

			atlas.size.y = std::max(atlas.size.y, allocation.reference.size.y);
		}

		uint64_t allocationSize = atlas.size.x * atlas.size.y * stride;
		atlas.data = (uint8_t*)std::malloc(allocationSize);
		VIVIUM_ASSERT(atlas.data != nullptr, "Failed allocation for stitched atlas of size {}", allocationSize);

		uint64_t atlasRow = stride * atlas.size.x;

		for (uint64_t i = 0; i < atlasCreator.allocations.size(); i++) {
			StitchedAtlasAllocation const& allocation = atlasCreator.allocations[i];

			uint64_t allocationRow = stride * allocation.reference.size.x;
			uint64_t allocationOffset = stride * allocation.reference.offset.x;

			// TODO: memcpy allocation into correct part of atlas
			for (uint64_t y = 0; y < allocation.reference.size.y; y++) {
				std::memcpy(
					atlas.data + atlasRow * y + allocationOffset,
					allocation.data + allocationRow * y,
					allocationRow
				);
			}
		}

		return atlas;
	}

	void dropAtlasCreator(StitchedAtlasCreator& atlasCreator)
	{
		for (uint64_t i = 0; i < atlasCreator.allocations.size(); i++) {
			StitchedAtlasAllocation const& allocation = atlasCreator.allocations[i];

			std::free(allocation.data);
		}

		atlasCreator.allocations.clear();
	}

	AtlasIndex convertStitchedAtlasReference(StitchedAtlasReference reference, StitchedAtlas const& atlas)
	{
		// TODO: check y math
		return textureAtlasIndex(atlas.size, I32x2(1), reference.size, reference.offset + reference.size);
	}

	void dropAtlas(StitchedAtlas& atlas)
	{
		std::free(atlas.data);
		VIVIUM_DEBUG_ONLY(atlas.data = nullptr);
	}

	AtlasIndex _calculateAtlasIndex(int left, int right, int bottom, int top, I32x2 atlasSize, I32x2 spriteSize) {
		AtlasIndex atlasIndex;

		float inverseWidth = 1.0f / atlasSize.x;
		float inverseHeight = 1.0f / atlasSize.y;

		// Calculate texture coordinates
		atlasIndex.left = left * inverseWidth * spriteSize.x;
		atlasIndex.right = right * inverseWidth * spriteSize.x;
		// Vertical flip here
		atlasIndex.bottom = top * inverseHeight * spriteSize.y;
		atlasIndex.top = bottom * inverseHeight * spriteSize.y;

		atlasIndex.translation = F32x2(atlasIndex.left, atlasIndex.top);
		atlasIndex.scale = F32x2(atlasIndex.right - atlasIndex.left, atlasIndex.bottom - atlasIndex.top);

		return atlasIndex;
	}

	AtlasIndex textureAtlasIndex(I32x2 atlasSize, I32x2 spriteSize, int index) {
		I32x2 atlasSprites = atlasSize / spriteSize;
		int yIndex = index / atlasSprites.y;
		int xIndex = index - yIndex * atlasSprites.x;

		return _calculateAtlasIndex(xIndex, xIndex + 1, yIndex, yIndex + 1, atlasSize, spriteSize);
	}

	AtlasIndex textureAtlasIndex(I32x2 atlasSize, I32x2 spriteSize, I32x2 index) {
		return _calculateAtlasIndex(index.x, index.x + 1, index.y, index.y + 1, atlasSize, spriteSize);
	}

	AtlasIndex textureAtlasIndex(I32x2 atlasSize, I32x2 spriteSize, I32x2 topLeft, I32x2 bottomRight) {
		return _calculateAtlasIndex(topLeft.x, bottomRight.x + 1, bottomRight.y, topLeft.y + 1, atlasSize, spriteSize);
	}
}