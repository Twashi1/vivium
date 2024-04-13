#include "font.h"

namespace Vivium {
	namespace Font {
		void init()
		{
			if (FT_Init_FreeType(&ftLibrary))
				VIVIUM_LOG(Log::FATAL, "Failed to initialise FreeType library");
		}
		
		void terminate()
		{
			FT_Done_FreeType(ftLibrary);
		}

		void computeSignedDistanceField(const uint8_t* input, uint64_t inputWidth, uint64_t inputHeight, uint8_t* output, uint64_t outputWidth, uint64_t outputHeight, float spreadFactor)
		{
			// Compute whether each pixel is in/out
			// Compute distance (within spread factor range) to both in/out pixel
			// Create signed distance field

			// TODO: offset with

			const uint64_t subpixelWidth = inputWidth / outputWidth;
			const uint64_t subpixelHeight = inputHeight / outputHeight;
			const uint8_t outThreshold = 0x80;

			for (uint64_t outY = 0; outY < outputHeight; outY++) {
				for (uint64_t outX = 0; outX < outputWidth; outX++) {

					uint64_t inputX = outX * subpixelWidth;
					uint64_t inputY = outY * subpixelHeight;

					uint64_t outputIndex = outX + outY * outputWidth;
					
					const int64_t centerX = inputX + subpixelWidth / 2;
					const int64_t centerY = inputY + subpixelHeight / 2;
					
					// Determine whether "in" or "out"
					//	samples the midpoint
					// TODO: multisampling approach would be better in future
					bool pixelIn = input[inputX + inputY * inputWidth] > outThreshold;

					// Now computing the signed distance field
					// TODO: for now setting strict radius limit at the subpixel size
					const int radius = 32;

					float minimumDistance = FLT_MAX;

					// TODO: perform a circular search outwards, ideally we want to search in "rings" until we
					//			find a texel of opposing colour, then we make spread factor independent of search radius
					for (int dy = -radius; dy <= radius; dy++) {
						// OOB check on Y
						int64_t testY = static_cast<int64_t>(inputY) + dy;

						if (testY < 0 || testY >= inputHeight) continue;

						for (int dx = -radius; dx <= radius; dx++) {
							// OOB check on X
							int64_t testX = static_cast<int64_t>(inputX) + dx;

							if (testX < 0 || testX >= inputWidth) continue;

							// Get pixel value
							uint8_t value = input[testX + testY * inputWidth];

							// Check pixel is of opposite polarity
							bool pixelPolarity = value > outThreshold;

							// It is of opposite polarity, compute distance
							if (pixelPolarity != pixelIn) {
								// Compute distance (Chebyshev)
								float distance = std::max(std::abs(testX - centerX), std::abs(testY - centerY));
								// Compute distance (Euclidean)
								// F32x2 pixelToMidpoint = F32x2(testX - centerX, testY - centerY);
								// float distance = pixelToMidpoint.length();

								minimumDistance = std::min(distance, minimumDistance);
							}
						}
					}

					float scaledDistance = minimumDistance / spreadFactor;
					if (!pixelIn) scaledDistance = -scaledDistance;

					// Clamp and convert to u8
					// TODO: better name?
					uint8_t alphaDistance = static_cast<uint8_t>(std::clamp(scaledDistance + 0.5f, 0.0f, 1.0f) * 0xff);

					// Write to output
					output[outputIndex] = alphaDistance;
				}
			}
		}

		void compileSignedDistanceField(const char* inputFontFile, int inputFontSize, uint8_t* outputDistanceField, int outputFieldSize, float spreadFactor)
		{
			FT_Face face;

			if (FT_Error error = FT_New_Face(ftLibrary, inputFontFile, 0, &face))
				VIVIUM_LOG(Log::FATAL, "Failed to load font at {}, error: {}", inputFontFile, error);

			FT_Set_Pixel_Sizes(face, 0, inputFontSize);

			uint64_t size = VIVIUM_CHARACTERS_TO_EXTRACT * outputFieldSize * outputFieldSize;

			I32x2 imageDimensions = I32x2(VIVIUM_CHARACTERS_TO_EXTRACT * outputFieldSize, outputFieldSize);

			uint8_t* glyphPixels = new uint8_t[inputFontSize * inputFontSize];
			uint8_t* glyphDistanceField = new uint8_t[outputFieldSize * outputFieldSize];

			for (uint8_t character = 0; character < VIVIUM_CHARACTERS_TO_EXTRACT; character++) {
				// TODO: FT_LOAD_RENDER probably a bad flag
				if (FT_Load_Char(face, character, FT_LOAD_RENDER)) {
					VIVIUM_LOG(Log::ERROR, "Failed to extract character {} from font", character);

					continue;
				}

				uint32_t glyphWidth = face->glyph->bitmap.width;
				uint32_t glyphHeight = face->glyph->bitmap.rows;

				uint64_t glyphIndex = 0;
				for (uint64_t i = 0; i < inputFontSize * inputFontSize; i++)
					glyphPixels[i] = 0x0;

				// Dealing with the | character which is larger than the font size for some reason
				if (glyphHeight >= inputFontSize) glyphHeight = inputFontSize;

				VIVIUM_ASSERT(glyphWidth <= inputFontSize && glyphHeight <= inputFontSize, "Font size not sufficient to fit glyph?");

				for (uint32_t y = 0; y < glyphHeight; y++) {
					for (uint32_t x = 0; x < glyphWidth; x++) {
						glyphPixels[x + y * inputFontSize] = face->glyph->bitmap.buffer[x + y * glyphWidth];
					}
				}

				// Compute distance field
				computeSignedDistanceField(glyphPixels, inputFontSize, inputFontSize, glyphDistanceField, outputFieldSize, outputFieldSize, spreadFactor);

				// Write back into texture atlas
				// Horizontal offset for each character
				uint64_t characterBufferOffset = character * outputFieldSize;

				for (uint64_t fieldY = 0; fieldY < outputFieldSize; fieldY++) {
					for (uint64_t fieldX = 0; fieldX < outputFieldSize; fieldX++) {
						uint8_t source = glyphDistanceField[fieldX + fieldY * outputFieldSize];

						outputDistanceField[characterBufferOffset + fieldX + fieldY * imageDimensions.x] = source;
					}
				}
			}

			delete[] glyphPixels;
			delete[] glyphDistanceField;

			FT_Done_Face(face);
		}

		Character getCharacter(Handle handle, uint8_t character)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(handle);

			return handle->characters[character];
		}

		int getFontSize(Handle handle)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(handle);

			return handle->fontSize;
		}

		const std::span<const uint8_t> getPixels(Handle handle)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(handle);

			return std::span<const uint8_t>(handle->pixels, handle->pixelsSize);
		}

		I32x2 getImageDimensions(Handle handle)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(handle);

			return handle->imageDimensions;
		}
		
		Specification Specification::fromFile(const char* filename, int fontSize)
		{
			Specification specification;

			FT_Face face;

			specification.fontSize = fontSize;

			if (FT_Error error = FT_New_Face(ftLibrary, filename, 0, &face))
				VIVIUM_LOG(Log::FATAL, "Failed to load font at {}, error: {}", filename, error);

			FT_Set_Pixel_Sizes(face, 0, fontSize);

			specification.pixelsSize = VIVIUM_CHARACTERS_TO_EXTRACT * fontSize * fontSize * 4;
			specification.pixels = new uint8_t[specification.pixelsSize];

			specification.imageDimensions = I32x2(VIVIUM_CHARACTERS_TO_EXTRACT * fontSize, fontSize);
			specification.atlas = TextureAtlas(specification.imageDimensions, I32x2(fontSize));

			uint64_t bufferOffset = 0;

			for (uint8_t character = 0; character < VIVIUM_CHARACTERS_TO_EXTRACT; character++) {
				// TODO: FT_LOAD_RENDER probably a bad flag
				if (FT_Load_Char(face, character, FT_LOAD_RENDER)) {
					VIVIUM_LOG(Log::ERROR, "Failed to extract character {} from font", character);
					
					continue;
				}

				// https://snorristurluson.github.io/TextRenderingWithFreetype/
				uint32_t glyphWidth = face->glyph->bitmap.width;
				uint32_t glyphHeight = face->glyph->bitmap.rows;

				for (uint32_t y = 0; y < glyphHeight; y++) {
					// TODO: each x layer should be convertible into a memcpy
					for (uint32_t x = 0; x < glyphWidth; x++) {
						uint8_t value = face->glyph->bitmap.buffer[x + y * glyphWidth];
						uint64_t index = y * (VIVIUM_CHARACTERS_TO_EXTRACT * fontSize) + bufferOffset + x;

						specification.pixels[index] = value;
					}
				}

				bufferOffset += fontSize;

				TextureAtlas::Index index = TextureAtlas::Index(character, specification.atlas);

				// TODO: constructor
				specification.characters[character] = Character{
					I32x2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
					I32x2(face->glyph->bitmap_left, face->glyph->bitmap_top),
					face->glyph->advance.x >> 6,
					index.left,
					index.left + face->glyph->bitmap.width / static_cast<float>(specification.imageDimensions.x),
					index.top + face->glyph->bitmap.rows / static_cast<float>(specification.imageDimensions.y),
					index.top
				};
			}

			FT_Done_Face(face);

			return specification;
		}
		
		void Specification::drop()
		{
			delete[] pixels;
		}

		Resource::Resource()
			: pixels(nullptr) {}

		bool Resource::isNull() const
		{
			return pixels == nullptr;
		}

		void Resource::create(Specification specification)
		{
			pixels = new uint8_t[specification.pixelsSize];
			std::memcpy(pixels, specification.pixels, specification.pixelsSize);

			pixelsSize = specification.pixelsSize;

			imageDimensions = specification.imageDimensions;
			fontSize = specification.fontSize;

			atlas = specification.atlas;
			characters = specification.characters;
		}

		void Resource::drop()
		{
			delete[] pixels;
		}
	}
}