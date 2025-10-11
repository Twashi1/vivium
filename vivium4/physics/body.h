#pragma once

#include "../math/vec2.h"
#include "material.h"
#include "shape.h"

namespace Vivium {
namespace Physics {
struct Body {
  F32x2 position, velocity, force;
  float angle, angularVelocity, torque;
  float inverseInertia, inverseMass;

  Shape shape;
  Material material;

  /*! \brief Apply impulse and a rotational force.
   *
   * \param impulse The impulse to apply to linear velocity.
   * \param vector The torque to apply to angular velocity.
   */
  void addImpulse(F32x2 impulse, F32x2 vector);

  bool enabled;
};
}  // namespace Physics
}  // namespace Vivium
