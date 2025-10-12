#pragma once

#include <format>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "../time/timer.h"

// TODO: prefer Rust type errors i think, but see what happens when we actually
// need errors

namespace Vivium {
namespace Error {
enum Result { FILE_NOT_FOUND, UNKNOWN, NONE };

const char* getResultName(Result type) {
  switch (type) {
    case FILE_NOT_FOUND:
      return "FILE_NOT_FOUND";
      break;
    case UNKNOWN:
      return "UNKNOWN";
      break;
    case NONE:
      return "NONE";
      break;
    default:
      return "INVALID";
      break;
  }
}
}  // namespace Error
}  // namespace Vivium