#include "math.h"
#include "../window.h"

namespace Vivium {
	namespace Math {
		Perspective orthogonalPerspective2D(F32x2 windowDimensions, F32x2 position, float rotation, float scale)
		{
			Perspective perspective;

			perspective.projection = glm::ortho(0.0f, windowDimensions.x, windowDimensions.y, 0.0f, -1.0f, 1.0f);
			glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(scale))
				* glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1))
				* glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f));
			perspective.view = glm::inverse(transform);

			return perspective;
		}

		// TODO: move to its own header?
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
}