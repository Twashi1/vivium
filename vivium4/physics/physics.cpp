#include "physics.h"

namespace Vivium {
	namespace Physics {
		PenetrationManifold::PenetrationManifold()
			: depth(0.0f), vector(0.0f), contacts({F32x2(0.0f), F32x2(0.0f)}), contactCount(0)
		{}
		
		EdgeManifold axisOfLeastPenetration(const Math::Polygon& a, const Math::Polygon& b, const Math::Transform& aTransform, const Math::Transform& bTransform)
		{
			float maximumDistance = std::numeric_limits<float>::lowest();
			uint64_t maximumIndex = 0;

			for (uint64_t i = 0; i < a.vertices.size(); i++) {
				F32x2 normalA = a.normals[i];
				F32x2 rotatedNormalA = aTransform.rotation * normalA;
				F32x2 bSpaceNormalA = bTransform.rotationInverse * rotatedNormalA;
				F32x2 support = b.support(-bSpaceNormalA);
				F32x2 vertex = Math::unapplyTransform(Math::applyTransform(a.vertices[i], aTransform), bTransform);

				float penetrationDistance = bSpaceNormalA.dot(support - vertex);

				if (penetrationDistance > maximumDistance) {
					maximumDistance = penetrationDistance;
					maximumIndex = i;
				}
			}

			return EdgeManifold{ maximumIndex, maximumDistance };
		}
		
		std::array<F32x2, 2> getIncidentFace(const Math::Polygon& reference, const Math::Polygon& incident, const Math::Transform& referenceTransform, const Math::Transform& incidentTransform, uint64_t referenceIndex)
		{
			F32x2 incidentSpaceReferenceNormal = incidentTransform.rotationInverse * (referenceTransform.rotation * reference.normals[referenceIndex]);

			float minimumProjection = std::numeric_limits<float>::max();
			uint64_t minimumIndex = 0;

			for (uint64_t i = 0; i < incident.vertices.size(); i++) {
				float dot = incidentSpaceReferenceNormal.dot(incident.normals[i]);

				if (dot < minimumProjection) {
					minimumProjection = dot;
					minimumIndex = i;
				}
			}

			std::array<F32x2, 2> faceVertices;
			faceVertices[0] = Math::applyTransform(incident.vertices[minimumIndex], incidentTransform);
			faceVertices[1] = Math::applyTransform(
				incident.vertices[
					minimumIndex == incident.vertices.size() - 1 ? 0 : minimumIndex + 1
				], incidentTransform
			);
	
			return faceVertices;
		}
		
		uint64_t clip(F32x2 edgeVector, float side, std::array<F32x2, MAX_CONTACT_COUNT>& face)
		{
			// TODO: cleanup, better outIndex

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
			if (distanceFace0 * distanceFace1 < 0.0f) {
				float alpha = distanceFace0 / (distanceFace0 - distanceFace1);
				out[outIndex] = face[0] + alpha * (face[1] - face[0]);
				++outIndex;
			}

			face[0] = out[0];
			face[1] = out[1];

			// TODO: better error message
			if (outIndex == 3)
				VIVIUM_LOG(Log::FATAL, "Out index was 3");

			return outIndex;
		}
		
		PenetrationManifold polygonToPolygon(const Math::Polygon& a, const Math::Polygon& b, const Math::Transform& aTransform, const Math::Transform& bTransform)
		{
			PenetrationManifold manifold;

			EdgeManifold edgeA = axisOfLeastPenetration(a, b, aTransform, bTransform);

			if (edgeA.depth >= 0.0f) return manifold;

			EdgeManifold edgeB = axisOfLeastPenetration(b, a, bTransform, aTransform);

			if (edgeB.depth >= 0.0f) return manifold;

			const Math::Polygon* reference;
			const Math::Polygon* incident;
			const Math::Transform* referenceTransform;
			const Math::Transform* incidentTransform;
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

			F32x2 referenceFaceVertex0 = Math::applyTransform(reference->vertices[referenceManifold->edgeIndex], *referenceTransform);
			F32x2 referenceFaceVertex1 = Math::applyTransform(reference->vertices[referenceManifold->edgeIndex == reference->vertices.size() - 1 ? 0 : referenceManifold->edgeIndex + 1], *referenceTransform);

			F32x2 referenceFaceVector = (referenceFaceVertex1 - referenceFaceVertex0).normalise();
			F32x2 referenceFaceNormal = referenceFaceVector.left();

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
		
		bool broadCollisionCheck(Body a, Body b)
		{
			// If either is disabled, they are not colliding
			if ((!a.enabled) || (!b.enabled)) return false;

			return Math::AABBIntersectAABB(
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
			Math::Transform transformA;
			transformA.position = a.position;
			transformA.rotation = Math::Mat2x2::fromAngle(a.angle);
			transformA.rotationInverse = transformA.rotation.transpose();

			Math::Transform transformB;
			transformB.position = b.position;
			transformB.rotation = Math::Mat2x2::fromAngle(b.angle);
			transformB.rotationInverse = transformB.rotation.transpose();

			// Perform SAT collision check
			// TODO: in future use jump table
			const Math::Polygon* polyA = reinterpret_cast<const Math::Polygon*>(a.shape.shape);
			const Math::Polygon* polyB = reinterpret_cast<const Math::Polygon*>(b.shape.shape);

			PenetrationManifold manifold = polygonToPolygon(*polyA, *polyB, transformA, transformB);

			if (manifold.contactCount == 0) return;

			// If they both have infinite mass, set velocity to 0 and exit
			if (a.inverseMass == 0.0f && b.inverseMass == 0.0f) {
				a.velocity = b.velocity = F32x2(0.0f);

				return;
			}

			// Otherwise, resolve collision
			for (uint64_t i = 0; i < manifold.contactCount; i++) {
				F32x2 contact = manifold.contacts[i];

				// TODO: these are constant between contacts
				float restitution = std::min(a.material.restitution, a.material.restitution);
				float staticFriction = std::sqrt(a.material.staticFriction * a.material.staticFriction);
				float dynamicFriction = std::sqrt(a.material.dynamicFriction * a.material.dynamicFriction);
				float contactCount = manifold.contactCount;
				float inverseContactCount = 1.0f / contactCount;

				// Vectors from bodies to points of contact
				F32x2 contactA = contact - a.position;
				F32x2 contactB = contact - b.position;

				// Relative velocity between bodies
				F32x2 relativeVelocity = b.velocity + b.angularVelocity * contactB.right()
					- a.velocity - a.angularVelocity * contactA.right();

				// Relative velocity in direction of collision normal
				float velocityLength = F32x2::dot(relativeVelocity, manifold.vector);

				// If moving away, don't bother resolving collision
				if (velocityLength > 0.0f) return;

				float velocityLengthA = F32x2::cross(contactA, manifold.vector);
				float velocityLengthB = F32x2::cross(contactB, manifold.vector);

				float inverseMassSum = a.inverseMass + b.inverseMass
					+ velocityLengthA * velocityLengthA * a.inverseInertia
					+ velocityLengthB * velocityLengthB * b.inverseInertia;

				float massSum = 1.0f / inverseMassSum;

				// Magnitude of reaction impulse
				// TODO: correct might be (-1.0f + restitution)
				float reactionLength = -(1.0f + restitution)
					* velocityLength
					* massSum
					* inverseContactCount;

				// Apply reaction impulse
				F32x2 reactionImpulse = manifold.vector * reactionLength;
				a.addImpulse(-reactionImpulse, contactA);
				b.addImpulse(reactionImpulse, contactB);

				// Re-calculate relative velocity for friction
				relativeVelocity = b.velocity + contactB.right() * b.angularVelocity
					- a.velocity - contactA.right() * a.angularVelocity;

				F32x2 contactTangent = F32x2::normalise(relativeVelocity - (manifold.vector
					* F32x2::dot(relativeVelocity, manifold.vector)));

				// Magnitude of friction impulse in direction of tangent
				float frictionLength = -F32x2::dot(relativeVelocity, contactTangent) * massSum
					* inverseContactCount;

				// If friction small, ignore
				// TODO: turn into constant EPSILON
				if (std::abs(frictionLength) < 0.0001f) return;

				F32x2 frictionImpulse;
				// If F < mu * R
				if (std::abs(frictionLength) < reactionLength * staticFriction) {
					frictionImpulse = contactTangent * frictionLength;
				}
				else {
					frictionImpulse = contactTangent * -reactionLength * dynamicFriction;
				}

				a.addImpulse(-frictionImpulse, contactA);
				b.addImpulse(frictionImpulse, contactB);
			}

			// Sinking correction
			const float strength = 0.4f; // Strength of sinking correction
			const float slop = 0.05f;	 // So bodies don't jitter with low penetrations

			F32x2 correction = (std::max(manifold.depth - slop, 0.0f) /
				(a.inverseMass + b.inverseMass))
				* strength
				* manifold.vector;

			a.position -= a.inverseMass * correction;
			b.position += b.inverseMass * correction;
		}
		
		void solve(std::span<Body> a, std::span<Body> b)
		{
			for (uint64_t i = 0; i < a.size(); i++) {
				// Start at i + 1 if intragroup, otherwise start at 0 for intergroup
				uint64_t startIndex = a.data() == b.data() ? i + 1 : 0;

				for (uint64_t j = startIndex; j < b.size(); j++) {
					checkCollisionAndResolve(a[i], b[j]);
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