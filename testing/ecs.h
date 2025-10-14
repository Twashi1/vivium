#pragma once

#include "shared.h"

namespace Testing {
struct ECSEnv {
  Registry* reg;
  Entity e;
};

void runECSTest();
}  // namespace Testing
