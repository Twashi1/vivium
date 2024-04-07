#pragma once

#include <array>
#include <bitset>

#include "../math/vec2.h"
#include "../math/aabb.h"
#include "body.h"
#include "material.h"
#include "shape.h"
#include "../math/transform.h"
#include "../core.h"

namespace Vivium {
	namespace Physics {
		// https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-friction-scene-and-jump-table--gamedev-7756?_ga=2.34091041.1492889901.1680594088-788194631.1680594088

		inline constexpr int MAX_CONTACT_COUNT = 2;

		struct PenetrationManifold {
			float depth;
			F32x2 vector;

			std::array<F32x2, MAX_CONTACT_COUNT> contacts;
			uint64_t contactCount;

			PenetrationManifold();
		};

		struct EdgeManifold {
			uint64_t edgeIndex;
			float depth;
		};

		EdgeManifold axisOfLeastPenetration(const Math::Polygon& a, const Math::Polygon& b, const Math::Transform& aTransform, const Math::Transform& bTransform);
	
		std::array<F32x2, 2> getIncidentFace(const Math::Polygon& reference, const Math::Polygon& incident, const Math::Transform& referenceTransform, const Math::Transform& incidentTransform, uint64_t referenceIndex);
	
		uint64_t clip(F32x2 edgeVector, float side, std::array<F32x2, MAX_CONTACT_COUNT>& face);

		PenetrationManifold polygonToPolygon(const Math::Polygon& a, const Math::Polygon& b, const Math::Transform& aTransform, const Math::Transform& bTransform);
	}
}