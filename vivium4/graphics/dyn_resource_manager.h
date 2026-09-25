#pragma once

#include "commands.h"
#include "primitives/buffer.h"
#include "primitives/descriptor_layout.h"
#include "primitives/descriptor_set.h"
#include "primitives/memory_type.h"
#include "primitives/pipeline.h"
#include "primitives/texture.h"
#include "resource_manager.h"

namespace Vivium {
struct DynamicResourceManager {
  std::vector<DeviceMemoryHandle> deviceMemoryHandles;
};
}  // namespace Vivium
