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

		// TODO: most of these should be listed as private functions to make it more clear
		//	what the user would typical use

		// Find axis of separation
		EdgeManifold axisOfLeastPenetration(const Polygon& a, const Polygon& b, const Transform& aTransform, const Transform& bTransform);
	
		std::array<F32x2, 2> getIncidentFace(const Polygon& reference, const Polygon& incident, const Transform& referenceTransform, const Transform& incidentTransform, uint64_t referenceIndex);
		uint64_t clip(F32x2 edgeVector, float side, std::array<F32x2, MAX_CONTACT_COUNT>& face);

		// Algorithm from: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-oriented-rigid-bodies--gamedev-8032t
		// Which in turn is based on: https://gdcvault.com/play/1017646/Physics-for-Game-Programmers-The
		// An advancement on the typical SAT method of projecting polygon extents onto each other for each axis
		PenetrationManifold polygonToPolygon(const Polygon& a, const Polygon& b, const Transform& aTransform, const Transform& bTransform);
	
		// Returns if two body AABBs are intersecting (broad phase collision check)
		bool broadCollisionCheck(Body const& a, Body const& b);
		// Check if two objects are colliding (narrow AND broad!), if so, resolve the collision
		void checkCollisionAndResolve(Body& a, Body& b);
		// Solve all collisions between two groups of bodies
		void solve(std::span<Body*> a, std::span<Body*> b);

		// TODO: automatic way to update bodies? pretty easy to forget
		void update(Body& body, float deltaTime);
	}
}