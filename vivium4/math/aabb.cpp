#include "aabb.h"

namespace Vivium {
	bool pointInAABB(F32x2 point, F32x2 min, F32x2 max) {
		return min.x <= point.x && max.x >= point.x && min.y <= point.y && max.y >= point.y;
	}

	// TODO: check this AABB check actually makes sense
	bool AABBIntersectAABB(F32x2 min1, F32x2 max1, F32x2 min2, F32x2 max2) {
		return (min1.x <= max2.x || min2.x >= max1.x) && (min1.y <= max2.y || min2.y >= max1.y);
	}
		
	F32x2 applyTransform(F32x2 point, Transform transform)
	{
		return transform.rotation * point + transform.position;
	}
		
	F32x2 unapplyTransform(F32x2 point, Transform transform)
	{
		return transform.rotationInverse * (point - transform.position);
	}
}