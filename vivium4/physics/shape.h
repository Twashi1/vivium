#pragma once

#include "../math/polygon.h"

namespace Vivium {
	namespace Physics {
		struct Shape {
			enum Type {
				UNKNOWN,
				POLYGON
			};

			Type type;
			const void* shape;

			Shape() = default;
			Shape(const Math::Polygon* polygon);
		};
	}
}