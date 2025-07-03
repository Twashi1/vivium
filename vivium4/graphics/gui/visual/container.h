#pragma once
// TODO: in visual but not actually a visual element

#include "context.h"
#include "../../color.h"
#include "../../../error/log.h"

namespace Vivium {
	enum class ContainerOrdering {
		NONE,
		VERTICAL,
		HORIZONTAL
	};

	enum class OffsetMethod {
		EXTENT,
		IMMEDIATE_CHILD
	};

	struct Container {
		GUIElementReference base;
	};

	struct ContainerSpecification {
		GUIElementReference parent;
		ContainerOrdering ordering;
		OffsetMethod offsetMethod;
	};

	Container createContainer(GUIContext& guiContext, ContainerSpecification specification);
}