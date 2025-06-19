#include "font.h"

namespace Vivium {
	void _fontInit()
	{
		if (FT_Init_FreeType(&ftLibrary))
			VIVIUM_LOG(LogSeverity::FATAL, "Failed to initialise FreeType library");
	}
		
	void _fontTerminate()
	{
		FT_Done_FreeType(ftLibrary);
	}

	void _computeSignedDistanceField(const uint8_t* input, uint64_t inputWidth, uint64_t inputHeight, uint8_t* output, uint64_t outputWidth, uint64_t outputHeight, float spreadFactor)
	{
		// Compute whether each pixel is in/out
		// Compute distance (within spread factor range) to both in/out pixel
		// Create signed distance field

		// TODO: offset glyphs on bottom as well

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
				const int radius = 64;

				// Spiral search from second
				//	not completely accurate since we use Euclidean distance, but stop when we find minimum
				//	Chebyshev distance (I think)

				float minimumDistance = FLT_MAX;
				int currentX = centerX;
				int currentY = centerY;
				int direction = 1;
				int moves = 1;

				while (moves < radius) {
					for (int dx = 0; dx < moves; dx++) {
						if (currentX + direction >= inputWidth || currentX + direction < 0) break;

						if (pixelIn != (input[currentX + currentY * inputWidth] > outThreshold))
						{
							minimumDistance = std::sqrt(
								(currentX - centerX) * (currentX - centerX) +
								(currentY - centerY) * (currentY - centerY)
							);

							goto foundMinimumDistance;
						}

						currentX += direction;
					}

					for (int dy = 0; dy < moves; dy++) {
						if (currentY + direction >= inputHeight || currentY + direction < 0) break;
							
						if (pixelIn != (input[currentX + currentY * inputWidth] > outThreshold))
						{
							minimumDistance = std::sqrt(
								(currentX - centerX) * (currentX - centerX) +
								(currentY - centerY) * (currentY - centerY)
							);

							goto foundMinimumDistance;
						}

						currentY += direction;
					}

					++moves;
					direction = -direction;
				}

				/*
				// Search square of radius
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
							// float distance = std::max(std::abs(testX - centerX), std::abs(testY - centerY));
							// Compute distance (Euclidean)
							F32x2 pixelToMidpoint = F32x2(testX - centerX, testY - centerY);
							float distance = pixelToMidpoint.length();

							// We don't have to check anything else, since we're searching circularly
							minimumDistance = std::min(distance, minimumDistance);
						}
					}
				}
				*/

			foundMinimumDistance:
				float scaledDistance = minimumDistance / radius * spreadFactor;
				if (!pixelIn) scaledDistance = -scaledDistance;

				// Clamp and convert to u8
				// TODO: better name?
				uint8_t alphaDistance = static_cast<uint8_t>(std::clamp(scaledDistance + 0.5f, 0.0f, 1.0f) * 0xff);

				// Write to output
				output[outputIndex] = alphaDistance;
			}
		}
	}

	// TODO: lots of similarity in functionality
	Font compileSignedDistanceField(const char* inputFontFile, int inputFontSize, const char* outputFile, int outputFieldSize, float spreadFactor)
	{
		// TODO: shouldn't pad the glpyh, only the field, so we need some extra math in computing the signed distance field on a glyph
			
		Font font;

		FT_Face face;

		if (FT_Error error = FT_New_Face(ftLibrary, inputFontFile, 0, &face))
			VIVIUM_LOG(LogSeverity::FATAL, "Failed to load font at {}, error: {}", inputFontFile, error);

		FT_Set_Pixel_Sizes(face, 0, inputFontSize);

		// TODO: parameterised padding
		int padding = outputFieldSize * 0.3f;
		int halfPadding = padding / 2;
		int glyphPaddedSize = inputFontSize + padding;
		int fieldPaddedSize = outputFieldSize + padding;

		VIVIUM_LOG(LogSeverity::DEBUG, "Padding is: {}", padding);

		font.fontSize = fieldPaddedSize;
		font.imageDimensions = I32x2(VIVIUM_CHARACTERS_TO_EXTRACT * fieldPaddedSize, fieldPaddedSize);
		font.fontSpriteSize = I32x2(fieldPaddedSize);
		font.data = std::vector<uint8_t>(font.imageDimensions.x * font.imageDimensions.y, 0);

		uint8_t* glyphPixels = reinterpret_cast<uint8_t*>(std::malloc(glyphPaddedSize * glyphPaddedSize));
		uint8_t* glyphDistanceField = reinterpret_cast<uint8_t*>(std::malloc(fieldPaddedSize * fieldPaddedSize));

		for (uint8_t character = 0; character < VIVIUM_CHARACTERS_TO_EXTRACT; character++) {
			// TODO: FT_LOAD_RENDER probably a bad flag
			if (FT_Load_Char(face, character, FT_LOAD_RENDER)) {
				VIVIUM_LOG(LogSeverity::ERROR, "Failed to extract character {} from font", character);

				continue;
			}

			uint32_t glyphWidth = face->glyph->bitmap.width;
			uint32_t glyphHeight = face->glyph->bitmap.rows;

			for (uint64_t i = 0; i < glyphPaddedSize * glyphPaddedSize; i++) {
				glyphPixels[i] = 0;
			}

			uint64_t glyphIndex = 0;

			// Deal with characters that might be larger than font size including padding
			if (glyphHeight + padding >= glyphPaddedSize) glyphHeight = glyphPaddedSize - padding;
			if (glyphWidth + padding >= glyphPaddedSize) glyphWidth = glyphPaddedSize - padding;

			for (uint32_t y = halfPadding; y < glyphHeight + halfPadding; y++) {
				for (uint32_t x = halfPadding; x < glyphWidth + halfPadding; x++) {
					glyphPixels[x + y * glyphPaddedSize] = face->glyph->bitmap.buffer[(x - halfPadding) + (y - halfPadding) * glyphWidth];
				}
			}

			// Compute distance field
			_computeSignedDistanceField(glyphPixels, glyphPaddedSize, glyphPaddedSize, glyphDistanceField, fieldPaddedSize, fieldPaddedSize, spreadFactor);

			// Write back into texture atlas
			// Horizontal offset for each character
			uint64_t characterBufferOffset = character * fieldPaddedSize;

			for (uint64_t fieldY = 0; fieldY < fieldPaddedSize; fieldY++) {
				for (uint64_t fieldX = 0; fieldX < fieldPaddedSize; fieldX++) {
					uint8_t source = glyphDistanceField[fieldX + fieldY * fieldPaddedSize];

					font.data[characterBufferOffset + fieldX + fieldY * font.imageDimensions.x] = source;
				}
			}

			AtlasIndex index = textureAtlasIndex(font.imageDimensions, font.fontSpriteSize, character);

			// Scaling from glyph parameters to field parameters
			float scaleRatio = static_cast<float>(fieldPaddedSize) / static_cast<float>(glyphPaddedSize);
			// Scaling from field parameters to UV coordinates
			F32x2 uvRatio = F32x2(1.0f) / font.imageDimensions;

			I32x2 fieldSize = F32x2(glyphWidth, glyphHeight) * scaleRatio + I32x2(halfPadding);
			I32x2 fieldBearing = F32x2(face->glyph->bitmap_left, face->glyph->bitmap_top) * scaleRatio;
			int fieldAdvance = (static_cast<int>(face->glyph->advance.x * scaleRatio) >> 6) + halfPadding;

			F32x2 uvSize = F32x2(fieldSize) * uvRatio;

			font.characters[character] = FontCharacter{
				fieldSize,
				fieldBearing,
				fieldAdvance,
				index.left,
				index.left + uvSize.x,
				index.top + uvSize.y,
				index.top
			};
		}

		// TODO: remove
		stbi_write_png("testGame/res/font.png", font.imageDimensions.x, font.imageDimensions.y, 1, font.data.data(), font.imageDimensions.x);

		std::free(glyphPixels);
		std::free(glyphDistanceField);

		FT_Done_Face(face);

		writeDistanceFieldFont(outputFile, font);

		return font;
	}

	void writeDistanceFieldFont(const char* outputFontFile, Font font)
	{
		std::fstream outputFile;

		outputFile.open(outputFontFile, std::ios::binary | std::ios::out);

		// Write magic text
		outputFile.write("sdf", 3);
		// Write font size
		outputFile.write(reinterpret_cast<const char*>(&font.fontSize), sizeof(int));
		// Write atlas
		outputFile.write(reinterpret_cast<const char*>(&font.fontSpriteSize), sizeof(I32x2));
		// Write image dimensions
		outputFile.write(reinterpret_cast<const char*>(&font.imageDimensions), sizeof(I32x2));
		// Write character table
		outputFile.write(reinterpret_cast<const char*>(font.characters.data()), font.characters.size() * sizeof(FontCharacter));
		// Write distance field size
		uint64_t fontDataSize = font.data.size();
		outputFile.write(reinterpret_cast<const char*>(&fontDataSize), sizeof(uint64_t));
		// Write distance field
		outputFile.write(reinterpret_cast<const char*>(font.data.data()), font.data.size());

		// End write
		outputFile.close();
	}
		
	Font createFontFile(const char* filename, int fontSize)
	{
		Font font;

		FT_Face face;

		font.fontSize = fontSize;

		if (FT_Error error = FT_New_Face(ftLibrary, filename, 0, &face))
			VIVIUM_LOG(LogSeverity::FATAL, "Failed to load font at {}, error: {}", filename, error);

		FT_Set_Pixel_Sizes(face, 0, fontSize);

		// TODO: reason for the * 4?
		font.data = std::vector<uint8_t>(VIVIUM_CHARACTERS_TO_EXTRACT * fontSize * fontSize * 4, 0);
		font.imageDimensions = I32x2(VIVIUM_CHARACTERS_TO_EXTRACT * fontSize, fontSize);
		font.fontSpriteSize = I32x2(fontSize);

		uint64_t bufferOffset = 0;

		for (uint8_t character = 0; character < VIVIUM_CHARACTERS_TO_EXTRACT; character++) {
			// TODO: FT_LOAD_RENDER probably a bad flag
			if (FT_Load_Char(face, character, FT_LOAD_RENDER)) {
				VIVIUM_LOG(LogSeverity::ERROR, "Failed to extract character {} from font", character);
					
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

					font.data[index] = value;
				}
			}

			bufferOffset += fontSize;

			AtlasIndex index = textureAtlasIndex(font.imageDimensions, font.fontSpriteSize, character);

			// TODO: constructor
			font.characters[character] = FontCharacter{
				I32x2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				I32x2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x >> 6,
				index.left,
				index.left + face->glyph->bitmap.width / static_cast<float>(font.imageDimensions.x),
				index.top + face->glyph->bitmap.rows / static_cast<float>(font.imageDimensions.y),
				index.top
			};
		}

		FT_Done_Face(face);

		return font;
	}

	Font createFontDistanceField(const char* filename)
	{
		Font font;
		std::fstream inputFile;

		inputFile.open(filename, std::ios::binary | std::ios::in);

		// Read magic text
		char magic[3];
		inputFile.read(magic, 3);

		VIVIUM_ASSERT(magic[0] == 's' && magic[1] == 'd' && magic[2] == 'f', "Expected magic at start of distance field file");

		inputFile.read(reinterpret_cast<char*>(&font.fontSize), sizeof(int));
		inputFile.read(reinterpret_cast<char*>(&font.fontSpriteSize), sizeof(I32x2));
		inputFile.read(reinterpret_cast<char*>(&font.imageDimensions), sizeof(I32x2));
		inputFile.read(reinterpret_cast<char*>(font.characters.data()), font.characters.size() * sizeof(FontCharacter));
		// Read distance field size
		uint64_t fontDataSize;
		inputFile.read(reinterpret_cast<char*>(&fontDataSize), sizeof(uint64_t));
		// Allocate space for distance field
		font.data = std::vector<uint8_t>(fontDataSize);

		// Read distance field
		inputFile.read(reinterpret_cast<char*>(font.data.data()), font.data.size());

		// End write
		inputFile.close();

		return font;
	}
}