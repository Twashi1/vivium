#pragma once

#include "../core.h"
#include "vec2.h"

namespace Vivium {
struct Perspective {
  glm::mat4 view;
  glm::mat4 projection;
};

/*! \brief Get the matrices for representing an orthogonal perspective.
 * \return The view and projection matrices.
 */
Perspective orthogonalPerspective2D(F32x2 frameDimensions, F32x2 position,
                                    float rotation, float scale);

template <typename T>
T nearestMultiple(T number, T multiple) {
  return (number + multiple - 1) & (-multiple);
}
}  // namespace Vivium
