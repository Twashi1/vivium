#include "shape.h"

namespace Vivium {
	namespace Physics {
		Shape::Shape(const Math::Polygon* polygon)
			: shape(polygon), type(Type::POLYGON)
		{}
	}
}