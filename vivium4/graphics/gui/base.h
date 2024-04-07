#pragma once

#include <span>
#include <vector>

#include "../../math/vec2.h"
#include "../../error/log.h"

namespace Vivium {
	namespace GUI {
		enum class PositionType {
			RELATIVE, // Offset to parent
			FIXED	  // Fixed
		};

		enum class ScaleType {
			FIXED,		// Consider scaling to be fixed number of pixels regardless of window dimensions
			VIEWPORT,	// Percentage of viewport dimensions
			RELATIVE	// Relative to parent
		};

		struct Properties {
			F32x2 dimensions;
			F32x2 position;

			F32x2 truePosition;
			F32x2 trueDimensions;

			PositionType positionType;
			ScaleType scaleType;
		};

		struct Base {
			Properties properties;

			std::vector<Base*> children;

			void propagateUpdate(F32x2 windowDimensions, Base* parent);
		};

		template <typename T>
		concept GUIElement = requires(T element) {
			{ element.base } -> std::same_as<Base&>;
		};
	}
}