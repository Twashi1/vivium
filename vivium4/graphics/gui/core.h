#pragma once

#include "../../math/vec2.h"

namespace Vivium {
	namespace GUI {
		enum class Position {
			RELATIVE,
			ABSOLUTE
		};

		enum class Scale {
			RELATIVE,
			ABSOLUTE
		};

		// Generate appropriate x and y widths such that it gives a square
		F32x2 Square(float length);
	}
}