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

		Character getCharacter(Handle handle, uint8_t character);
		int getFontSize(Handle handle);
		const std::span<const uint8_t> getPixels(Handle handle);
		I32x2 getImageDimensions(Handle handle);
	}
}