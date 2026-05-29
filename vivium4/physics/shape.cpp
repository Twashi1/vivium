#include "shape.h"

namespace Vivium {
namespace Physics {
F32x2 Shape::getMin() const {
  switch (type) {
    case Type::POLYGON:
      return reinterpret_cast<const Polygon*>(shape)->min;
      break;
    default:
      VIVIUM_LOG(LogSeverity::FATAL, "Invalid shape type");
      break;
  }

  return F32x2(0.0f);
}

F32x2 Shape::getMax() const {
  switch (type) {
    case Type::POLYGON:
      return reinterpret_cast<const Polygon*>(shape)->max;
      break;
    default:
      VIVIUM_LOG(LogSeverity::FATAL, "Invalid shape type");
      break;
  }

  return F32x2(0.0f);
}

Shape::Shape(const Polygon* polygon) : shape(polygon), type(Type::POLYGON) {}
}  // namespace Physics
}  // namespace Vivium
