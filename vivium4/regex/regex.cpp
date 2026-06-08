#include "regex.h"

namespace Vivium {
Match match(Pattern const& pattern, char const* input) {
  VIVIUM_ASSERT(input != nullptr, "Input was nullptr");

  // TODO: lookup optimisations for regex parsers
  // probably very many to make it faster

  uint32_t maxLengthMatch = 0;

  // TODO: other notes; we only support a full-match with this current pattern
  // TODO: we need a full set of accepting states
  // TODO: we then need to re-run the algorithm for every index of the input
  // - we can make some minor optimisations: if the state 0 was available at
  // some index i, we do not have to re-run that index
  std::vector<uint8_t> possibleStates = std::vector<uint8_t>(pattern.numStates);
  possibleStates[0] = 1;

  for (uint32_t i = 0; input[i] != '\0'; i++) {
    char c = input[i];
    uint32_t charID = pattern.charToID[c];

    // TODO: dense array might be useful optimisation
    for (uint32_t k = 0; k < possibleStates.size(); k++) {
      if (!possibleStates[k]) continue;

      // decrement this state as we transition out of it
      --possibleStates[k];

      // assuming we consume the character c does place a cost on us
      // in terms of creating the pattern
      // we have to DFS every epsilon transition
      uint32_t tableIndex = charID * pattern.numUniqueChars + k;

      std::vector<uint32_t> const& nextStates =
          pattern.transitionTable[tableIndex];

      for (uint32_t j = 0; j < nextStates.size(); j++) {
        uint32_t nextState = nextStates[j];

        // saturation
        possibleStates[nextState] =
            std::max(possibleStates[nextState] + 1, 255);

        if (pattern.isStateAccepting[nextState]) {
          maxLengthMatch = i;
        }
      }
    }
  }

  Match match;
  match.matchStart = 0;
  match.matchLength = maxLengthMatch;

  return match;
}
}  // namespace Vivium
