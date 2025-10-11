#pragma once

#include "../math/polygon.h"

namespace Vivium {
namespace Physics {
struct Shape {
  enum Type { POLYGON };

  Type type;
  const void* shape;

  /*! \brief Get the minimum of the bounding box. */
  F32x2 getMin() const;
  /*! \brief Get the maximum of the bounding box. */
  F32x2 getMax() const;

  Shape() = default;
  Shape(const Polygon* polygon);
};
}  // namespace Physics
}  // namespace Vivium
