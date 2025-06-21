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

	struct Container {
		GUIElementReference base;
	};

	struct ContainerSpecification {
		GUIElementReference parent;
		ContainerOrdering ordering;
	};

	Container createContainer(GUIContext& guiContext, ContainerSpecification specification);
}