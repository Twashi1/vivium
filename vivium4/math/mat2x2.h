#pragma once

#include "vec2.h"

namespace Vivium {
	namespace Math {
		struct Mat2x2 {
			float m00, m01;
			float m10, m11;

			Mat2x2(float m00, float m01, float m10, float m11)
				: m00(m00), m01(m01), m10(m10), m11(m11)
			{}

			friend F32x2 operator*(Mat2x2 matrix, F32x2 vec) {
				return F32x2(
					matrix.m00 * vec.x + matrix.m10 * vec.y,
					matrix.m01 * vec.x + matrix.m11 * vec.y
				);
			}

			Mat2x2 transpose() {
				return Mat2x2(m00, m10, m01, m11);
			}
		};
	}
}