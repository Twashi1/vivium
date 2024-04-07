#pragma once

namespace Vivium {
	namespace Physics {
		struct Material {
			float restitution;
			float staticFriction;
			float dynamicFriction;
			float density;

			static const Material Default;
		};
	}
}