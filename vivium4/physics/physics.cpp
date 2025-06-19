#include "physics.h"

namespace Vivium {
	namespace Physics {
		PenetrationManifold::PenetrationManifold()
			: depth(0.0f), vector(0.0f), contacts({F32x2(0.0f), F32x2(0.0f)}), contactCount(0)
		{}
		
		EdgeManifold axisOfLeastPenetration(const Polygon& a, const Polygon& b, const Transform& aTransform, const Transform& bTransform)
		{
			// For each face, find the support point in the direction of the negation of the normal to that face
			// i.e. the vertex furthest in a certain normal direction
			// The distance from each support point to the current face gives us the signed penetration
			// We select axis of largest signed distance -> least penetration (furthest away)
			// This algorithm will likely break if the shapes are deeply intersecting (about more than half?), but this isn't of much concern

			float maximumDistance = std::numeric_limits<float>::lowest();
			uint64_t maximumIndex = 0;

			for (uint64_t i = 0; i < a.vertices.size(); i++) {
				F32x2 normalA = a.normals[i];
				// Move normal into B model space
				F32x2 rotatedNormalA = aTransform.rotation * normalA;
				F32x2 bSpaceNormalA = bTransform.rotationInverse * rotatedNormalA;
				// Get B's support in the direction of the negation of this face of A's normal
				F32x2 support = supportPolygon(b, -bSpaceNormalA);
				// Transform vertex from A's model space into B's model space
				// A -> World -> B
				F32x2 vertex = unapplyTransform(applyTransform(a.vertices[i], aTransform), bTransform);

				// Calculate penetration distance in B's model space
				//	by projecting distance between support and one of the vertices of face
				//	onto the normal
				float penetrationDistance = F32x2::dot(bSpaceNormalA, support - vertex);

				if (penetrationDistance > maximumDistance) {
					maximumDistance = penetrationDistance;
					maximumIndex = i;
				}
			}

			return EdgeManifold{ maximumIndex, maximumDistance };
		}
		
		std::array<F32x2, 2> getIncidentFace(const Polygon& reference, const Polygon& incident, const Transform& referenceTransform, const Transform& incidentTransform, uint64_t referenceIndex)
		{
			F32x2 incidentSpaceReferenceNormal = incidentTransform.rotationInverse * (referenceTransform.rotation * reference.normals[referenceIndex]);

			float minimumProjection = std::numeric_limits<float>::max();
			uint64_t minimumIndex = 0;

			for (uint64_t i = 0; i < incident.vertices.size(); i++) {
				float dot = F32x2::dot(incidentSpaceReferenceNormal, incident.normals[i]);

				if (dot < minimumProjection) {
					minimumProjection = dot;
					minimumIndex = i;
				}
			}

			std::array<F32x2, 2> faceVertices;
			faceVertices[0] = applyTransform(incident.vertices[minimumIndex], incidentTransform);
			faceVertices[1] = applyTransform(
				incident.vertices[
					minimumIndex == incident.vertices.size() - 1 ? 0 : minimumIndex + 1
				], incidentTransform
			);
	
			return faceVertices;
		}
		
		uint64_t clip(F32x2 edgeVector, float side, std::array<F32x2, MAX_CONTACT_COUNT>& face)
		{
			uint32_t outIndex = 0;
			F32x2 out[2] = {
				face[0],
				face[1]
			};

			// Distances from each endpoint to the line
			float distanceFace0 = F32x2::dot(edgeVector, face[0]) - side;
			float distanceFace1 = F32x2::dot(edgeVector, face[1]) - side;

			// If behind plane
			if (distanceFace0 <= 0.0f) out[outIndex++] = face[0];
			if (distanceFace1 <= 0.0f) out[outIndex++] = face[1];

			// If they have opposite sign/different side of plane
			// outIndex < 2 is just sanity check, not mathematically possible for
			//	distance faces to have different signs and outIndex >= 2
			if (outIndex < 2 && distanceFace0 * distanceFace1 < 0.0f) {
				float alpha = distanceFace0 / (distanceFace0 - distanceFace1);
				out[outIndex] = face[0] + alpha * (face[1] - face[0]);
				++outIndex;
			}

			face[0] = out[0];
			face[1] = out[1];

			return outIndex;
		}
		
		PenetrationManifold polygonToPolygon(const Polygon& a, const Polygon& b, const Transform& aTransform, const Transform& bTransform)
		{
			PenetrationManifold manifold;

			EdgeManifold edgeA = axisOfLeastPenetration(a, b, aTransform, bTransform);

			if (edgeA.depth >= 0.0f) return manifold;

			EdgeManifold edgeB = axisOfLeastPenetration(b, a, bTransform, aTransform);

			if (edgeB.depth >= 0.0f) return manifold;

			const Polygon* reference;
			const Polygon* incident;
			const Transform* referenceTransform;
			const Transform* incidentTransform;
			bool flip;
			EdgeManifold* referenceManifold;

			// TODO: check if even needed
			auto biasGreater = [](float a, float b) {
				return a >= b * 0.95f + a * 0.01f;
			};

			if (biasGreater(edgeA.depth, edgeB.depth)) {
				reference = &a;
				incident = &b;
				referenceTransform = &aTransform;
				incidentTransform = &bTransform;
				referenceManifold = &edgeA;
				flip = false;
			}
			else {
				reference = &b;
				incident = &a;
				referenceTransform = &bTransform;
				incidentTransform = &aTransform;
				referenceManifold = &edgeB;
				flip = true;
			}

			std::array<F32x2, 2> incidentFace = getIncidentFace(*reference, *incident, *referenceTransform, *incidentTransform, referenceManifold->edgeIndex);

			F32x2 referenceFaceVertex0 = applyTransform(reference->vertices[referenceManifold->edgeIndex], *referenceTransform);
			F32x2 referenceFaceVertex1 = applyTransform(reference->vertices[referenceManifold->edgeIndex == reference->vertices.size() - 1 ? 0 : referenceManifold->edgeIndex + 1], *referenceTransform);

			F32x2 referenceFaceVector = F32x2::normalise(referenceFaceVertex1 - referenceFaceVertex0);
			F32x2 referenceFaceNormal = F32x2::left(referenceFaceVector);

			float referenceClipped = F32x2::dot(referenceFaceNormal, referenceFaceVertex0);
			float negativeVector = -F32x2::dot(referenceFaceVector, referenceFaceVertex0);
			float positiveVector = F32x2::dot(referenceFaceVector, referenceFaceVertex1);

			if (clip(-referenceFaceVector, negativeVector, incidentFace) < 2) return manifold;
			if (clip(referenceFaceVector, positiveVector, incidentFace) < 2) return manifold;

			manifold.vector = flip ? -referenceFaceNormal : referenceFaceNormal;

			uint64_t clippedPoints = 0;

			float separation = F32x2::dot(referenceFaceNormal, incidentFace[0]) - referenceClipped;

			if (separation <= 0.0f) {
				manifold.contacts[clippedPoints++] = incidentFace[0];
				manifold.depth = -separation;
			}

			separation = F32x2::dot(referenceFaceNormal, incidentFace[1]) - referenceClipped;

			if (separation <= 0.0f) {
				manifold.contacts[clippedPoints++] = incidentFace[1];
				manifold.depth += -separation;
				manifold.depth /= static_cast<float>(clippedPoints);
			}

			manifold.contactCount = clippedPoints;

			return manifold;
		}
		
		bool broadCollisionCheck(Body const& a, Body const& b)
		{
			// If either is disabled, they are not colliding
			if ((!a.enabled) || (!b.enabled)) return false;

			return AABBIntersectAABB(
				a.shape.getMin() + a.position,
				a.shape.getMax() + a.position,
				b.shape.getMin() + b.position,
				b.shape.getMax() + b.position
			);
		}

		void checkCollisionAndResolve(Body& a, Body& b)
		{
			// If broad phase check doesn't pass, exit
			if (!broadCollisionCheck(a, b)) return;

			// Generate transforms from body
			Transform transformA;
			transformA.position = a.position;
			transformA.rotation = Mat2x2::fromAngle(a.angle);
			transformA.rotationInverse = transformA.rotation.transpose();

			Transform transformB;
			transformB.position = b.position;
			transformB.rotation = Mat2x2::fromAngle(b.angle);
			transformB.rotationInverse = transformB.rotation.transpose();

			// Perform SAT collision check
			// TODO: in future use jump table
			const Polygon* polyA = reinterpret_cast<const Polygon*>(a.shape.shape);
			const Polygon* polyB = reinterpret_cast<const Polygon*>(b.shape.shape);

			PenetrationManifold manifold = polygonToPolygon(*polyA, *polyB, transformA, transformB);

			if (manifold.contactCount == 0) return;

			// If they both have infinite mass, set velocity to 0 and exit
			if (a.inverseMass == 0.0f && b.inverseMass == 0.0f) {
				a.velocity = b.velocity = F32x2(0.0f);

				return;
			}

			// Compute some constants
			// Bouncy * not bouncy = not bouncy, bouncy * bouncy = very bouncy, not bouncy * not bouncy = not bouncy
			float restitution = a.material.restitution * b.material.restitution;
			// Approximations, bias towards both values being high
			float staticFriction = std::sqrt(a.material.staticFriction * b.material.staticFriction);
			float dynamicFriction = std::sqrt(a.material.dynamicFriction * b.material.dynamicFriction);
			float contactCount = manifold.contactCount;
			float inverseContactCount = 1.0f / contactCount;

			// Resolve
			for (uint64_t i = 0; i < manifold.contactCount; i++) {
				F32x2 contact = manifold.contacts[i];

				// Vectors from bodies to points of contact
				F32x2 contactA = contact - a.position;
				F32x2 contactB = contact - b.position;

				// Relative velocity between bodies projected along normals to contact
				F32x2 relativeVelocity = b.velocity + b.angularVelocity * F32x2::right(contactB)
					- a.velocity - a.angularVelocity * F32x2::right(contactA);

				// Relative velocity in direction of collision normal
				float velocityLength = F32x2::dot(relativeVelocity, manifold.vector);

				// If moving away, don't bother resolving collision
				if (velocityLength > 0.0f) return;

				float velocityLengthA = F32x2::cross(contactA, manifold.vector);
				float velocityLengthB = F32x2::cross(contactB, manifold.vector);

				// TODO: introduced inertia, better name?
				float inverseMassSum = a.inverseMass + b.inverseMass
					+ velocityLengthA * velocityLengthA * a.inverseInertia
					+ velocityLengthB * velocityLengthB * b.inverseInertia;

				float massSum = 1.0f / inverseMassSum;

				// Magnitude of reaction impulse
				float reactionLength = -(1.0f + restitution)
					* velocityLength
					* massSum
					* inverseContactCount;

				// Apply reaction impulse
				F32x2 reactionImpulse = manifold.vector * reactionLength;
				a.addImpulse(-reactionImpulse, contactA);
				b.addImpulse(reactionImpulse, contactB);

				// Re-calculate relative velocity for friction
				relativeVelocity = b.velocity + F32x2::right(contactB) * b.angularVelocity
					- a.velocity - F32x2::right(contactA) * a.angularVelocity;

				// Project collision normal onto velocity (length), and multiply collision normal by length, then
				//	take the tangent as the norm of the scaled collision normal to the relative velocity from A to B
				//	This tangent is perpendicular to the normal closest to the relative velocity (so is further from relative velocity)
				F32x2 contactTangent = F32x2::normalise(relativeVelocity - (manifold.vector
					* F32x2::dot(relativeVelocity, manifold.vector)));

				// Magnitude of friction impulse in direction of tangent
				// Friction is applied opposite to the tangent vector, hence negation
				float frictionLength = -F32x2::dot(relativeVelocity, contactTangent) * massSum
					* inverseContactCount;

				// If friction small, ignore
				// TODO: turn into constant EPSILON
				if (std::abs(frictionLength) < 0.0001f) return;

				F32x2 frictionImpulse;
				// Coulomb's law
				// Clamp magnitude below some threshold, "if slow moving, do static, if fast moving, do dynamic"
				// If F < mu * R, resolve static friction
				if (std::abs(frictionLength) < reactionLength * staticFriction) {
					frictionImpulse = contactTangent * frictionLength;
				}
				// Resolve dynamic friction
				else {
					frictionImpulse = contactTangent * -reactionLength * dynamicFriction;
				}

				a.addImpulse(-frictionImpulse, contactA);
				b.addImpulse(frictionImpulse, contactB);
			}

			// Sinking correction (Linear projection to resolve)
			// Just move each object (position) along the collision normal by a small amount
			//	Correction is (inversely) proportional to mass of each object
			const float strength = 0.4f; // Strength of sinking correction
			// Only consider objects with adequately small penetration depth
			const float slop = 0.05f;

			F32x2 correction = (std::max(manifold.depth - slop, 0.0f) /
				(a.inverseMass + b.inverseMass))
				* strength
				* manifold.vector;

			a.position -= a.inverseMass * correction;
			b.position += b.inverseMass * correction;
		}
		
		void solve(std::span<Body*> a, std::span<Body*> b)
		{
			for (uint64_t i = 0; i < a.size(); i++) {
				// Start at i + 1 if intragroup, otherwise start at 0 for intergroup
				uint64_t startIndex = a.data() == b.data() ? i + 1 : 0;

				for (uint64_t j = startIndex; j < b.size(); j++) {
					checkCollisionAndResolve(*a[i], *b[j]);
				}
			}
		}
		
		void update(Body& body, float deltaTime)
		{
			if (body.inverseMass == 0.0f) return;

			body.velocity += body.force * body.inverseMass * deltaTime;
			body.position += body.velocity * deltaTime;

			body.angularVelocity += body.torque * body.inverseInertia * deltaTime;
			body.angle += body.angularVelocity * deltaTime;

			body.force = F32x2(0.0f);
			body.torque = 0.0f;
		}
	}
}