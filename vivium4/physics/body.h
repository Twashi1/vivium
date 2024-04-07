#pragma once

#include "../math/vec2.h"

namespace Vivium {
	namespace Physics {
		struct Body {
			F32x2 position, velocity, force;
			float angle, angularVelocity, torque;
			float inverseInertia, inverseMass;

			bool enabled;
		};
	}
}