#include "math.h"

namespace Vivium {
	namespace Math {
		Perspective orthogonalPerspective2D(Window::Handle window, F32x2 position, float rotation, float scale)
		{
			I32x2 dimensions = Window::dimensions(window);

			Perspective perspective;

			perspective.projection = glm::ortho(0.0f, static_cast<float>(dimensions.x), static_cast<float>(dimensions.y), 0.0f, -1.0f, 1.0f);
			glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(scale))
				* glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1))
				* glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f));
			perspective.view = glm::inverse(transform);

			return perspective;
		}
		
		uint64_t calculateAlignmentOffset(uint64_t& currentOffset, uint64_t size, uint64_t alignment)
		{
			uint64_t resourceOffset = nearestMultiple(currentOffset, alignment);
			uint64_t requiredPadding = resourceOffset - currentOffset;
			currentOffset += requiredPadding + size;

			// Return the offset the resource should be placed at
			return resourceOffset;
		}
	}
}