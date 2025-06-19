#pragma once

#include "vec2.h"

namespace Vivium {
	struct Mat2x2 {
		float m00, m01;
		float m10, m11;

		Mat2x2() = default;
		Mat2x2(float m00, float m01, float m10, float m11);

		// TODO: don't make this a child function?
		friend F32x2 operator*(Mat2x2 matrix, F32x2 vec);

		Mat2x2 transpose();

		static Mat2x2 fromAngle(float angle);
		static Mat2x2 identity();
	};
}