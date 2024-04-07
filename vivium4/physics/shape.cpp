#include "shape.h"

namespace Vivium {
	namespace Physics {
		F32x2 Shape::getMin() const
		{
			switch (type) {
			case Type::POLYGON: return reinterpret_cast<const Math::Polygon*>(shape)->min;
			default: VIVIUM_LOG(Log::FATAL, "Invalid shape type");
			}
		}

		F32x2 Shape::getMax() const
		{
			switch (type) {
			case Type::POLYGON: return reinterpret_cast<const Math::Polygon*>(shape)->max;
			default: VIVIUM_LOG(Log::FATAL, "Invalid shape type");
			}
		}

		Shape::Shape(const Math::Polygon* polygon)
			: shape(polygon), type(Type::POLYGON)
		{}
	}
}