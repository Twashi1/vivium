#pragma once

#include "shared.h"

namespace Testing {
struct ECSEnv {
  Registry* reg;
};

void runECSTest();
}  // namespace Testing
