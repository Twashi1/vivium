#pragma once

#include "vec2.h"
#include "../core.h"

namespace Vivium {
	namespace Window {
		struct Resource;

		typedef Resource* Handle;
	}

	namespace Math {
		struct Perspective {
			glm::mat4 view;
			glm::mat4 projection;
		};

		Perspective orthogonalPerspective2D(Window::Handle window, F32x2 position, float rotation, float scale);

		template <typename T>
		T nearestMultiple(T number, T multiple) {
			return (number + multiple - 1) & (-multiple);
		}

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

		// TODO: move to its own header?
		AtlasIndex _calculateAtlasIndex(int left, int right, int bottom, int top, I32x2 atlasSize, I32x2 spriteSize);

		AtlasIndex textureAtlasIndex(I32x2 atlasSize, I32x2 spriteSize, int index);
		AtlasIndex textureAtlasIndex(I32x2 atlasSize, I32x2 spriteSize, I32x2 index);
		AtlasIndex textureAtlasIndex(I32x2 atlasSize, I32x2 spriteSize, I32x2 topLeft, I32x2 bottomRight);
	}
}