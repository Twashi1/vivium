#include "math.h"

namespace Vivium {
Perspective orthogonalPerspective2D(F32x2 windowDimensions, F32x2 position,
                                    float rotation, float scale) {
  Perspective perspective;

  perspective.projection = glm::ortho(0.0f, windowDimensions.x,
                                      windowDimensions.y, 0.0f, -1.0f, 1.0f);
  glm::mat4 transform =
      glm::scale(glm::mat4(1.0f), glm::vec3(scale)) *
      glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1)) *
      glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f));
  perspective.view = glm::inverse(transform);

  return perspective;
}

uint64_t integerPower(uint64_t base, uint64_t exp) {
  uint64_t res = 1;

  while (exp) {
    if (exp & 1) res *= base;
    exp >>= 1;
    base *= base;
  }

  return res;
}
}  // namespace Vivium
