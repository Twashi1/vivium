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
	}
}