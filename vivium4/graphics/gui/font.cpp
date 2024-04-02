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