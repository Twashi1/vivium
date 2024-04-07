#pragma once

#include "vec2.h"
#include "mat2x2.h"

namespace Vivium {
	namespace Math {
		struct Transform {
			F32x2 position;
			Mat2x2 rotation;
			Mat2x2 rotationInverse;
		};
	}
}