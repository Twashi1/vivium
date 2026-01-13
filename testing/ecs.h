#pragma once

#include "shared.h"

namespace Testing {
struct ECSEnv {
  Registry* reg;
  Entity e;

  View<Owned<int>, Partial<float>> basicView;
  std::vector<Entity> viewEntities;
};

void runECSTest();
}  // namespace Testing
