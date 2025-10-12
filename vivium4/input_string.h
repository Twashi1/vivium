#pragma once

#include "input.h"

// TODO: re-implement input string class

namespace Vivium {
struct InputString {
  std::string text;
  uint64_t cursorPosition;

  uint64_t selectionStart;
  uint64_t selectionEnd;
};
}  // namespace Vivium
