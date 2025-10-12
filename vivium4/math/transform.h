#pragma once

#include "mat2x2.h"
#include "vec2.h"

namespace Vivium {
struct Transform {
  F32x2 position;
  Mat2x2 rotation;
  Mat2x2 rotationInverse;

  static Transform zero();
};
}  // namespace Vivium