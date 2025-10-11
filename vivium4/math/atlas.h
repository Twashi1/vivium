#pragma once

#include "../core.h"
#include "../graphics/image_load.h"
#include "../graphics/texture_format.h"
#include "vec2.h"

// TODO: move out of math path?
// Support for stitching multiple atlases together
//	when stitching multiple atlases together, it should return the atlas index
// for each stitched atlas 	most likely its possible to return a reference such
// that when the final atlas is stiched 	we can retrieve all relevant
// information (left/right/top/bottom) without having to store additional
// metadata

// https://www.david-colson.com/2020/03/10/exploring-rect-packing.html
// Note we don't even have full information beforehand, we have to greedily pack
// them
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
  uint64_t xOffset;  // Relatively naive approach to packing, just put it all in
                     // a row
};

// TODO: textures loading upside-down
struct StitchedAtlas {
  uint8_t* data;
  I32x2 size;
  TextureFormat format;
};

/*! \brief Create the tracking object for constructing a stitched atlas.
 *
 * \param format The texture format from which to construct the atlas.
 * \return The stitched atlas tracker.
 */
StitchedAtlasCreator createStitchedAtlasCreator(TextureFormat format);

/*! \brief Read an image, and add it to the atlas.
 *
 * \param imageFilename The image to open and read.
 * \param atlasCreator The stitched atlas to continue building.
 * \return A reference to the position of this image that can later be used for
 * retrieval.
 */
StitchedAtlasReference submitToStitchedAtlasCreator(
    const char* imageFilename, StitchedAtlasCreator& atlasCreator);
/*! \brief Given an image specification, and add it to the atlas.
 *
 * \param size The size of the image data to submit to the atlas.
 * \param data Raw image data to submit to the atlas.
 * \param atlasCreator The atlas to submit to.
 * \return A reference to the position of this image that can later be used for
 * retrieval.
 */
StitchedAtlasReference submitToStitchedAtlasCreator(
    I32x2 size, uint8_t* data, StitchedAtlasCreator& atlasCreator);
/*! \brief Finalise the atlas, validating all references and returning the atlas
 * data.
 *
 * Attempting to submit more data is undefined behaviour after this.
 *
 * \param atlasCreator The atlas to finalise.
 * \return The atlas with the image data.
 */
StitchedAtlas finishAtlasCreation(StitchedAtlasCreator const& atlasCreator);
/*! \brief Free the memory of the atlas creator.
 *
 * Does not require the atlas to be finalised.
 *
 * \param atlasCreator The atlas creator to destroy.
 */
void dropAtlasCreator(StitchedAtlasCreator& atlasCreator);
/*! \brief Given an atlas reference, get an actual atlas index. */
AtlasIndex convertStitchedAtlasReference(StitchedAtlasReference reference,
                                         StitchedAtlas const& atlas);
/*! \brief Free image data of the stitched atlas. */
void dropAtlas(StitchedAtlas& atlas);

// Assumes rectangle atlas with entries organised as a grid
AtlasIndex _calculateAtlasIndex(int left, int right, int bottom, int top,
                                I32x2 atlasSize, I32x2 spriteSize);
/*! \brief Get a texture atlas from a 1D index. */
AtlasIndex textureAtlasIndex(I32x2 atlasSize, I32x2 spriteSize, int index);
/*! \brief Get a texture atlas from a 2D index. */
AtlasIndex textureAtlasIndex(I32x2 atlasSize, I32x2 spriteSize, I32x2 index);
/*! \brief Get a texture atlas from a range of indices (inclusive). */
AtlasIndex textureAtlasIndex(I32x2 atlasSize, I32x2 spriteSize, I32x2 topLeft,
                             I32x2 bottomRight);
}  // namespace Vivium
