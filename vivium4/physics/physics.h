#pragma once

#include <array>
#include <bitset>

#include "../core.h"
#include "../math/aabb.h"
#include "../math/transform.h"
#include "../math/vec2.h"
#include "body.h"
#include "material.h"
#include "shape.h"

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

// TODO: most of these should be listed as private functions to make it more
// clear what the user would typical use

/*! \brief Find the axis to separate along/of least penetration.
 *
 * For each face, find the support point in the direction of the negation of
 * the normal to that face i.e. the vertex furthest in a certain normal
 * direction The distance from each support point to the current face gives us
 * the signed penetration We select axis of largest signed distance -> least
 * penetration (furthest away) This algorithm will likely break if the shapes
 * are deeply intersecting (about more than half?), but this isn't of much
 * concern
 *
 * \param a The first polygon.
 * \param b The second polygon.
 * \param aTransform The transformation to apply to the first polygon.
 * \param bTransform The transformation to apply to the second polygon.
 * \return Information about the edge of least penetration and the penetration
 * depth.
 */
EdgeManifold axisOfLeastPenetration(const Polygon& a, const Polygon& b,
                                    const Transform& aTransform,
                                    const Transform& bTransform);

/*! \brief Get the two vertices of the intersecting face of the incident.
 *
 * These contact points are transformed by the given transforms.
 *
 * \param reference The reference polygon.
 * \param incident The incident polygon.
 * \param referenceTransform The transformation to apply to the reference
 * polygon.
 * \param incidentTransform The transformation to apply to the incident polygon.
 * \return The transformed vertex points of the incident face.
 */
std::array<F32x2, 2> getIncidentFace(const Polygon& reference,
                                     const Polygon& incident,
                                     const Transform& referenceTransform,
                                     const Transform& incidentTransform,
                                     uint64_t referenceIndex);

// TODO: explanation of arguments
/*! \brief Get the contact points and number of contacts.
 *
 * \param edgeVector
 * \param side
 * \param face The contact points to fill in.
 * \return The number of contact points.
 */
uint64_t clip(F32x2 edgeVector, float side,
              std::array<F32x2, MAX_CONTACT_COUNT>& face);

/*! \brief Get the penetration between two polygons.
 *
 * Algorithm from:
 * https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-oriented-rigid-bodies--gamedev-8032t
 * Which in turn is based on:
 * https://gdcvault.com/play/1017646/Physics-for-Game-Programmers-The
 * An advancement on the typical SAT method of projecting polygon extents onto
 * each other for each axis
 *
 * \return Data describing the penetration depth, number of contacts, and the
 * vector for separation.
 */
PenetrationManifold polygonToPolygon(const Polygon& a, const Polygon& b,
                                     const Transform& aTransform,
                                     const Transform& bTransform);

/*! \brief Returns if two body AABBs are intersecting (broad phase collision
 * check) */
bool broadCollisionCheck(Body const& a, Body const& b);
/*! \brief Check if two objects are colliding and resolve the collision.
 *
 * Performs both broad and narrow phase checks before attempting to resolve.
 *
 * \param a First body to check.
 * \param b Second body to check.
 */
void checkCollisionAndResolve(Body& a, Body& b);
/*! \brief Solve all collisions between two groups of bodies.
 *
 * Both lists can be the same. This does not automatically update the bodies.
 *
 * \param a A list of bodies to check for collisions.
 * \param b A list of bodies to check for collisions.
 */
void solve(std::span<Body*> a, std::span<Body*> b);

// TODO: automatic way to update bodies? pretty easy to forget
/*! \brief Update the body given some timestep. */
void update(Body& body, float deltaTime);
}  // namespace Physics
}  // namespace Vivium
