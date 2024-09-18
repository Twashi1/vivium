#pragma once

#include <span>
#include <limits>
#include <vector>

#include "vec2.h"
#include "../core.h"
#include "transform.h"
#include "aabb.h"

namespace Vivium {
	namespace Math {
		struct Polygon {
			std::vector<F32x2> vertices;
			std::vector<F32x2> normals;

			F32x2 min;
			F32x2 max;
		};

		F32x2 centroidPolygon(Polygon const& polygon);
		F32x2 supportPolygon(Polygon const& polygon, F32x2 direction);
		float inertiaPolygon(Polygon const& polygon);
		float areaPolygon(Polygon const& polygon);
	
		bool containsPolygon(Polygon const& polygon, F32x2 point, Math::Transform transform);

		Polygon createPolygonVertices(const std::span<const F32x2> vertices);
		Polygon createPolygonRegular(float radius, uint64_t vertexCount);
		Polygon createPolygonBox(F32x2 dimensions);
	}
}