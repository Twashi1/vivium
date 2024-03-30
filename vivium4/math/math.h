#pragma once

#include "vec2.h"
#include "../window.h"
#include "../core.h"

namespace Vivium {
	namespace Math {
		struct Perspective {
			glm::mat4 projection;
			glm::mat4 view;
		};

		Perspective calculatePerspective(Window::Handle window, F32x2 position, float rotation, float scale);

		template <typename T>
		T nearestMultiple(T number, T multiple) {
			return (number + multiple - 1) & (-multiple);
		}

		uint64_t calculateAlignmentOffset(uint64_t& currentOffset, uint64_t size, uint64_t alignment);
	}
}