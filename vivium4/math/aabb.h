#pragma once

#include "transform.h"
#include "vec2.h"

namespace Vivium {

/*! \brief Check if a point is in an AABB. */
bool pointInAABB(F32x2 point, F32x2 min, F32x2 max);

// TODO: check this AABB check actually makes sense
/*! \brief Given two AABBs, check if they are intersecting. */
bool AABBIntersectAABB(F32x2 min1, F32x2 max1, F32x2 min2, F32x2 max2);

// TODO: these should not be in AABB
/*! \brief Apply a transform to a point and return it. */
F32x2 applyTransform(F32x2 point, Transform transform);
/*! \brief Apply the inverse of a transform to a point and return it. */
F32x2 unapplyTransform(F32x2 point, Transform transform);
}  // namespace Vivium
