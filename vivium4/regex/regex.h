#pragma once

#include "../core.h"
#include "../error/log.h"

namespace Vivium {
inline constexpr uint32_t EPSILON_TRANSITION = 0;

struct Pattern {
  // TODO: fast sparse map
  // note the inner vector signifies the set of possible states
  std::vector<std::vector<uint32_t>> transitionTable;
  std::vector<uint32_t> charToID;
  std::vector<uint32_t> isStateAccepting;
  uint32_t numUniqueChars;
  uint32_t numStates;
};

struct Match {
  uint32_t matchStart;
  uint32_t matchLength;
};

Match match(Pattern const& pattern, char const* input);
}  // namespace Vivium
