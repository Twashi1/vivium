#pragma once

#include "engine.h"

struct TreeContainer {
  // recursive structure?
  //	each leaf is a regular vertical container
  Container root;
  void* data;
  bool enabled;
  bool held;

  std::vector<TreeContainer> children;
};

bool operator==(TreeContainer const& a, TreeContainer const& b);

TreeContainer createTreeContainer(GUIContext& guiContext,
                                  GUIElementReference parent);
void addChild(TreeContainer& container, TreeContainer& child,
              GUIContext& guiContext);
void removeChild(TreeContainer& container, TreeContainer& child,
                 GUIContext& guiContext);
TreeContainer* getContainer(F32x2 position, TreeContainer& container,
                            GUIContext& guiContext);
TreeContainer* findParent(TreeContainer& container, TreeContainer& child,
                          GUIContext& guiContext);
void addNewChild(TreeContainer& container, void* data,
                 GUIElementReference reference, GUIContext& guiContext);

TreeContainer* updateTreeContainer(F32x2 cursorPosition,
                                   TreeContainer& container,
                                   TreeContainer* held, GUIContext& context);

namespace Vivium {
template <SerialiserInterface Store>
void serialiseWrite(TreeContainer const& container, Store& store) {
  // TODO: Assuming the data passed is POD... and is 4 bytes...
  //	really TreeContainer should jsut take an entity or something
  dispatchSerialiseWrite(*reinterpret_cast<int const*>(container.data), store);
  dispatchSerialiseWrite(container.enabled, store);
  dispatchSerialiseWrite(container.children, store);
}

template <SerialiserInterface Store>
void serialiseRead(TreeContainer* const container, Store& store) {
  dispatchSerialiseRead(reinterpret_cast<int* const>(container->data), store);
  dispatchSerialiseRead(&container->enabled, store);
  dispatchSerialiseRead(&container->children, store);
}
}  // namespace Vivium
