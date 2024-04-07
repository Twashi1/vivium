#include "polygon.h"
#include "polygon.h"

namespace Vivium {
	namespace Math {
		F32x2 Polygon::centroid() const
		{
			constexpr float third = 1.0f / 3.0f;

			F32x2 center = 0.0f;
			float area = 0.0f;

			for (uint64_t i = 0; i < vertices.size(); i++) {
				F32x2 current = vertices[i];
				F32x2 next = vertices[i == vertices.size() - 1 ? 0 : i + 1];

				// Triangle with current, next, and origin, assuming origin within triangle
				float triangleArea = std::abs(current.cross(next) * 0.5f);
				area += triangleArea;
				center += (current + next) * triangleArea * third;
			}

			return center / area;
		}

		F32x2 Polygon::support(F32x2 direction) const
		{
			float maxDotProduct = std::numeric_limits<float>::lowest();
			F32x2 bestPoint = vertices[0];

			for (uint64_t i = 1; i < vertices.size(); i++) {
				F32x2 point = vertices[i];

				float dot = point.dot(direction);

				if (dot > maxDotProduct) {
					bestPoint = point;
					maxDotProduct = dot;
				}
			}

			return bestPoint;
		}

		float Polygon::inertia() const
		{
			constexpr float twelth = 1.0f / 12.0f;

			float inertia = 0.0f;

			for (uint64_t i = 0; i < vertices.size(); i++) {
				F32x2 current = vertices[i];
				F32x2 next = vertices[i == vertices.size() - 1 ? 0 : i + 1];

				// TODO: check this is actually correct
				// https://physics.stackexchange.com/questions/708936/how-to-calculate-the-moment-of-inertia-of-convex-polygon-two-dimensions
				inertia += twelth * current.cross(next) * (
					current.dot(current) + next.dot(next) + current.dot(next)
				);
			}

			return inertia;
		}

		float Polygon::area() const
		{
			float area = 0.0f;

			for (uint64_t i = 0; i < vertices.size(); i++) {
				F32x2 current = vertices[i];
				F32x2 next = vertices[i == vertices.size() - 1 ? 0 : i + 1];

				float triangleArea = std::abs(current.cross(next) * 0.5f);

				area += triangleArea;
			}

			return area;
		}

		bool Polygon::contains(F32x2 point, Math::Transform transform)
		{
			F32x2 testPoint = Math::unapplyTransform(point, transform);

			// If not within AABB, early exit
			if (!Math::pointInAABB(point, min, max)) return false;

			int previousSideOrientation = 2;

			for (uint64_t i = 0; i < vertices.size(); i++) {
				F32x2 current = vertices[i];
				F32x2 next = vertices[i == vertices.size() - 1 ? 0 : i + 1];

				float orientation = F32x2::orient(current, next, point);
				// > 0 -> 1
				// < 0 -> -1
				// = 0 -> 0
				int sideOrientation = (orient > 0) - (orient < 0);

				// TODO: can simplify this logic probably (need a better way to iterate current and next vertex)
				// Take first non-zero side orientation when we don't have a side orientation
				if (previousSideOrientation == 2 && sideOrientation != 0) previousSideOrientation = sideOrientation;
				else if (previousSideOrientation == 0) continue;
				else if (previousSideOrientation != sideOrientation) return false;
			}

			return true;
		}

		static Polygon Polygon::fromVertices(const std::span<const F32x2> vertices)
		{
			// TODO: ensure vertices contain origin

			VIVIUM_ASSERT(vertices.size() >= 3, "Not enough vertices for valid polygon");

			Polygon polygon;

			polygon.vertices = vertices;
			polygon.normals.resize(vertices.size());
			polygon.min = polygon.vertices.front();
			polygon.max = polygon.vertices.back();

			VIVIUM_ASSERT(polygon.contains(F32x2(0.0f), Transform::zero()), "Vertices must contain origin");

			F32x2 center = polygon.centroid();

			for (F32x2& vertex : polygon.vertices)
				vertex -= center;

			for (uint64_t i = 0; i < polygon.vertices.size(); i++) {
				F32x2 current = polygon.vertices[i];
				
				// Update bounds
				if (current.x < polygon.min.x)
					polygon.min.x = current.x;
				else if (current.x > polygon.max.x)
					polygon.max.x = current.x;
				if (current.y < polygon.min.y)
					polygon.min.y = current.y;
				else if (current.y > polygon.max.y)
					polygon.max.y = current.y;

				F32x2 next = polygon.vertices[i == polygon.vertices.size() - 1 ? 0 : i + 1];

				// Assume counter-clockwise ordering
				polygon.normals[i] = (next - current).left().normalise();
			}

			return polygon;
		}

		static Polygon Polygon::fromRegular(float radius, uint64_t vertexCount)
		{
			std::vector<F32x2> vertices(vertexCount);

			float angle = 0.0f;

			constexpr float pi = 3.1415927f;

			float turn = 2.0f * pi / static_cast<float>(vertexCount);

			for (uint64_t i = 0; i < vertexCount; i++) {
				float cosAngle = std::cos(angle);
				float sinAngle = std::sin(angle);

				vertices[i] = F32x2(cosAngle, sinAngle) * radius;

				angle += turn;
			}

			return Polygon::fromVertices(vertices);
		}

		static Polygon Polygon::fromBox(F32x2 dimensions)
		{
			F32x2 halfdim = dimensions * 0.5f;
			float left   = -halfdim.x;
			float right  =  halfdim.x;
			float bottom = -halfdim.y;
			float top    =  halfdim.y;

			return Polygon::fromVertices(std::vector<F32x2>({
				F32x2(left, bottom),
				F32x2(right, bottom),
				F32x2(right, top),
				F32x2(left, top)
			}));
		}
	}
}