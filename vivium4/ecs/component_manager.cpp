#include "component_manager.h"

namespace Vivium {
uint64_t ComponentManager::getOffset(uint64_t index) const {
  return typeSize * index;
}
}  // namespace Vivium