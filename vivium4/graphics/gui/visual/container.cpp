#include "container.h"

namespace Vivium {
	Container createContainer(GUIContext& guiContext, ContainerSpecification specification)
	{
		Container container;

		GUIElementType containerType = GUIElementType::DEFAULT;

		switch (specification.ordering) {
		case ContainerOrdering::NONE: VIVIUM_LOG(Log::ERROR, "Container ordering must be vertical/horizontal"); break;
		case ContainerOrdering::VERTICAL: containerType = GUIElementType::CONTAINER_VERTICAL; break;
		case ContainerOrdering::HORIZONTAL: containerType = GUIElementType::CONTAINER_HORIZONTAL; break;
		}

		container.base = createGUIElement(guiContext, containerType);
		addChild(specification.parent, { &container.base, 1 }, guiContext);

		return container;
	}
}