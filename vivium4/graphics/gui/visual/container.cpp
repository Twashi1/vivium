#include "container.h"

namespace Vivium {
Container createContainer(GUIContext& guiContext,
                          ContainerSpecification specification) {
  Container container;

  container.base = createGUIElement(
      guiContext,
      _ContainerUpdateData(specification.ordering, specification.offsetMethod));
  addChild(specification.parent, {&container.base, 1}, guiContext);

  return container;
}
}  // namespace Vivium