#include "body.h"

namespace Vivium {
	namespace Physics {
		void Body::addImpulse(F32x2 impulse, F32x2 vector) {
			velocity += inverseMass * impulse;
			angularVelocity += inverseInertia * vector.cross(impulse);
		}
	}
}