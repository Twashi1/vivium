#pragma once

#include "../math/vec2.h"
#include "shape.h"
#include "material.h"

namespace Vivium {
	namespace Physics {
		struct Body {
			F32x2 position, velocity, force;
			float angle, angularVelocity, torque;
			float inverseInertia, inverseMass;

			Shape shape;
			Material material;

			void addImpulse(F32x2 impulse, F32x2 vector);

			bool enabled;
		};
	}
}