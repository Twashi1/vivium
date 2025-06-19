#pragma once

#include "../math/polygon.h"

namespace Vivium {
	namespace Physics {
		struct Shape {
			enum Type {
				POLYGON
			};

			Type type;
			const void* shape;

			F32x2 getMin() const;
			F32x2 getMax() const;

			Shape() = default;
			Shape(const Polygon* polygon);
		};
	}
}