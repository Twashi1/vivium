#pragma once

#include "engine.h"

struct TreeContainer {
	// recursive structure?
	//	each leaf is a regular vertical container
	Container root;
	void* data;
	bool enabled;
	
	std::vector<TreeContainer> children;
};

bool operator==(TreeContainer const& a, TreeContainer const& b);

TreeContainer createTreeContainer(GUIContext& guiContext, GUIElementReference parent);
void addChild(TreeContainer& container, TreeContainer& child, GUIContext& guiContext);
void removeChild(TreeContainer& container, TreeContainer& child, GUIContext& guiContext);
TreeContainer* getContainer(F32x2 position, TreeContainer& container, GUIContext& guiContext);
TreeContainer* findParent(TreeContainer& container, TreeContainer& child, GUIContext& guiContext);
void addNewChild(TreeContainer& container, void* data, GUIElementReference reference, GUIContext& guiContext);

TreeContainer* updateTreeContainer(F32x2 cursorPosition, TreeContainer& container, TreeContainer* held, GUIContext& context);
