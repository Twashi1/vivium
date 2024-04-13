#pragma once

#include <array>
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

		struct Specification {
			uint8_t* pixels;
			uint64_t pixelsSize;

			I32x2 imageDimensions;
			int fontSize;

			TextureAtlas atlas;

			std::array<Character, VIVIUM_CHARACTERS_TO_EXTRACT> characters;

			static Specification fromFile(const char* filename, int fontSize);

			// Drop the allocate pixels when creating from file
			void drop();
		};

		struct Resource {
			uint8_t* pixels;
			uint64_t pixelsSize;

			I32x2 imageDimensions;
			int fontSize;

			TextureAtlas atlas;

			std::array<Character, VIVIUM_CHARACTERS_TO_EXTRACT> characters;

			Resource();

			bool isNull() const;
			void create(Specification specification);
			void drop();
		};

		typedef Resource* Handle;

		template <Allocator::AllocatorType AllocatorType>
		Handle create(AllocatorType allocator, Specification specification)
		{
			Handle handle = Allocator::allocateResource<Resource>(allocator);
			
			handle->create(specification);

			return handle;
		}

		template <Allocator::AllocatorType AllocatorType>
		void drop(AllocatorType allocator, Handle handle)
		{
			VIVIUM_CHECK_HANDLE_EXISTS(handle);

			handle->drop();

			Allocator::dropResource(allocator, handle);
		}
		
		// https://cdn.akamai.steamstatic.com/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf
		// https://libgdx.com/wiki/graphics/2d/fonts/distance-field-fonts
		void computeSignedDistanceField(const uint8_t* input, uint64_t inputWidth, uint64_t inputHeight, uint8_t* output, uint64_t outputWidth, uint64_t outputHeight, float spreadFactor);
		void compileSignedDistanceField(const char* inputFontFile, int inputFontSize, uint8_t* outputDistanceField, int outputFieldsize, float spreadFactor);

		Character getCharacter(Handle handle, uint8_t character);
		int getFontSize(Handle handle);
		const std::span<const uint8_t> getPixels(Handle handle);
		I32x2 getImageDimensions(Handle handle);
	}
}