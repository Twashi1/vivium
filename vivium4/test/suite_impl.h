#pragma once

#include "suite.h"

namespace Vivium {
template <ValidTestFunctor Functor>
void pushTest(TestSuite& suite, std::string_view name, Functor testCode) {
  // Skip test if fatal error occurred
  if (suite.encounteredFatal) {
    return;
  }

  pushResult(suite, name, testCode());
}
}  // namespace Vivium
