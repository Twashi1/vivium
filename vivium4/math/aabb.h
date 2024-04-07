#pragma once

#include "vec2.h"
#include "transform.h"

namespace Vivium {
	namespace Math {
		bool pointInAABB(F32x2 point, F32x2 min, F32x2 max);

		// TODO: check this AABB check actually makes sense
		bool AABBIntersectAABB(F32x2 min1, F32x2 max1, F32x2 min2, F32x2 max2);

		F32x2 applyTransform(F32x2 point, Transform transform);
		F32x2 unapplyTransform(F32x2 point, Transform transform);
	}
}