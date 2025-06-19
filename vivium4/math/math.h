#pragma once

#include "vec2.h"
#include "../core.h"

namespace Vivium {
	struct Perspective {
		glm::mat4 view;
		glm::mat4 projection;
	};

	Perspective orthogonalPerspective2D(F32x2 frameDimensions, F32x2 position, float rotation, float scale);

	template <typename T>
	T nearestMultiple(T number, T multiple) {
		return (number + multiple - 1) & (-multiple);
	}
}