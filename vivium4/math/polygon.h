#pragma once

#include <limits>
#include <span>
#include <vector>

#include "../core.h"
#include "aabb.h"
#include "transform.h"
#include "vec2.h"

namespace Vivium {
struct Polygon {
  std::vector<F32x2> vertices;
  std::vector<F32x2> normals;

  F32x2 min;
  F32x2 max;
};

/*! \brief Calculate the centroid of the polygon.
 *
 * Equivalent to the center of mass in most convex shapes. Assuming uniform
 * density.
 *
 * \return The centroid.
 */
F32x2 centroidPolygon(Polygon const& polygon);
/*! \brief Get the support point in a normalised direction.
 *
 * Get the vertex that is most in the given direction. Assumes the direction is
 * already normalised.
 *
 * \return The support vertex.
 */
F32x2 supportPolygon(Polygon const& polygon, F32x2 direction);
/*! \brief Calculate the inertia of the polygon. */
float inertiaPolygon(Polygon const& polygon);
/*! \brief Get the area of a convex shape. */
float areaPolygon(Polygon const& polygon);

/*! \brief Returns if a polygon contains a point, given a transformation on the
 * polygon. */
bool containsPolygon(Polygon const& polygon, F32x2 point, Transform transform);

Polygon createPolygonVertices(const std::span<const F32x2> vertices);
Polygon createPolygonRegular(float radius, uint64_t vertexCount);
Polygon createPolygonBox(F32x2 dimensions);
}  // namespace Vivium
