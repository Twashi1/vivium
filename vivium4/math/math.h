#pragma once

#include "vec2.h"
#include "../window.h"
#include "../core.h"

namespace Vivium {
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
	}
}