#pragma once

#include <span>
#include <limits>
#include <vector>

#include "vec2.h"
#include "../core.h"

namespace Vivium {
	namespace Math {
		struct Polygon {
			std::vector<F32x2> vertices;
			std::vector<F32x2> normals;

			F32x2 min, max;

			F32x2 centroid() const;
			F32x2 support(F32x2 direction) const;

			float inertia() const;
			float area() const;

			Polygon() = default;
			
			static Polygon fromVertices(const std::span<const F32x2> vertices);
			static Polygon fromRegular(float radius, uint64_t vertexCount);
			static Polygon fromBox(F32x2 dimensions);
		};
	}
}