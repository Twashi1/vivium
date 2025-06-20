#include "shape.h"

namespace Vivium {
	namespace Physics {
		F32x2 Shape::getMin() const
		{
			switch (type) {
			case Type::POLYGON: return reinterpret_cast<const Polygon*>(shape)->min;
			default: VIVIUM_LOG(LogSeverity::FATAL, "Invalid shape type");
			}
		}

		F32x2 Shape::getMax() const
		{
			switch (type) {
			case Type::POLYGON: return reinterpret_cast<const Polygon*>(shape)->max;
			default: VIVIUM_LOG(LogSeverity::FATAL, "Invalid shape type");
			}
		}

		Shape::Shape(const Polygon* polygon)
			: shape(polygon), type(Type::POLYGON)
		{}
	}
}