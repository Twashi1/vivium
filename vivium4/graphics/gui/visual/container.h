#pragma once
// TODO: in visual but not actually a visual element

#include "../../../error/log.h"
#include "../../color.h"
#include "context.h"

namespace Vivium {
enum class ContainerOrdering { NONE, VERTICAL, HORIZONTAL };

enum class OffsetMethod { EXTENT, IMMEDIATE_CHILD };

struct Container {
  GUIElementReference base;
};

struct ContainerSpecification {
  GUIElementReference parent;
  ContainerOrdering ordering;
  OffsetMethod offsetMethod;
};

Container createContainer(GUIContext& guiContext,
                          ContainerSpecification specification);
}  // namespace Vivium