#include "tree_container.h"

bool operator==(TreeContainer const& a, TreeContainer const& b)
{
	return a.root.base == b.root.base;
}

TreeContainer createTreeContainer(GUIContext& guiContext, GUIElementReference parent) {
	TreeContainer tree;

	tree.root = createContainer(guiContext, ContainerSpecification(parent, ContainerOrdering::VERTICAL));
	properties(tree.root, guiContext).anchorY = GUIAnchor::TOP;
	properties(tree.root, guiContext).centerY = GUIAnchor::TOP;

	return tree;
}

void insertChild(TreeContainer& container, TreeContainer& child, uint64_t position, GUIContext& guiContext) {
	container.children.insert(container.children.begin() + position, child);
	insertChild(container.root.base, { &child.root.base, 1 }, position, guiContext);
}

void addChild(TreeContainer& container, TreeContainer& child, GUIContext& guiContext) {
	container.children.push_back(child);
	addChild(container.root.base, { &child.root.base, 1 }, guiContext);
}

void removeChild(TreeContainer& container, TreeContainer& child, GUIContext& guiContext)
{
	if (container == child) {
		VIVIUM_LOG(LogSeverity::WARN, "Cannot remove head of a tree container");
		
		return;
	}

	if (std::find(container.children.begin(), container.children.end(), child) != container.children.end()) {
		container.children.erase(std::remove(container.children.begin(), container.children.end(), child), container.children.end());
		removeChild(container.root.base, { &child.root.base, 1 }, guiContext);
	}
	else {
		for (TreeContainer& nextContainer : container.children) {
			removeChild(nextContainer, child, guiContext);
		}
	}
}

TreeContainer* getContainer(F32x2 position, TreeContainer& container, GUIContext& guiContext) {
	// TODO: if the container has children, we should be more specific in finding the exact child
	//	that we're hovering, rather than just the container
	// TODO: not sure this code is correct by above ^
	TreeContainer* result = nullptr;

	if (container.enabled && pointInExtent(position, properties(container.root, guiContext))) {
		result = &container;

		// Search children to attempt to get more specificity
		for (TreeContainer& child : container.children) {
			TreeContainer* childResult = getContainer(position, child, guiContext);

			if (childResult != nullptr) {
				return childResult;
			}
		}
	}

	return result;
}

TreeContainer* findParent(TreeContainer& container, TreeContainer& child, GUIContext& guiContext)
{
	if (std::find(container.children.begin(), container.children.end(), child) != container.children.end()) {
		return &container;
	}

	for (TreeContainer& nextContainer : container.children) {
		TreeContainer* possibleParent = findParent(nextContainer, child, guiContext);
		
		if (possibleParent != nullptr) return possibleParent;
	}

	return nullptr;
}

void addNewChild(TreeContainer& container, void* data, GUIElementReference reference, GUIContext& guiContext)
{
	TreeContainer newTree = createTreeContainer(guiContext, container.root.base);
	newTree.data = data;
	newTree.enabled = false;
	addChild(newTree.root.base, { &reference, 1 }, guiContext);

	// Add slight x offset
	properties(newTree.root, guiContext).position.x = 0.05f;
	
	container.children.push_back(newTree);
}

TreeContainer* updateTreeContainer(F32x2 cursorPosition, TreeContainer& container, TreeContainer* held, GUIContext& context)
{
	// TODO: should not be able to pick up the root
	// TODO: should be ensuring we can't make a loop
	// TODO: manually disable the root container?
	// TODO: detection code doesn't work great?
	//	probably won't work at all on child containers
	TreeContainer* hovered = getContainer(cursorPosition, container, context);

	if (hovered != nullptr) {
		GUIProperties& props = properties(hovered->root.base, context);
		debugRect(props.minExtent, props.maxExtent - props.minExtent, Color(1.0f, 0.0f, 0.0f), context);

		// VIVIUM_LOG(LogSeverity::DEBUG, "Hovering {} at {} {}", hovered->root.base.index, cursorPosition.x, cursorPosition.y);
	}

	if (hovered != nullptr && *hovered == container) {
		return held;
	}

	// If we're not holding anything, and we pressed left,
	//  then select this element
	if (hovered != nullptr && held == nullptr && Input::get(Input::BTN_1).state == Input::DOWN) {
		VIVIUM_LOG(LogSeverity::DEBUG, "Element is held {}", hovered->root.base.index);
		hovered->enabled = false;
		return hovered;
	}

	// If we're holding something, and we've released on something selected
	if (Input::get(Input::BTN_1).state == Input::UP && held != nullptr) {
		// If we released the held element on itself return
		if (held == hovered) {
			VIVIUM_LOG(LogSeverity::DEBUG, "Element is released on itself");
			return nullptr;
		}

		// If we released the held element on nothing return
		if (hovered == nullptr) {
			VIVIUM_LOG(LogSeverity::DEBUG, "Element is released on nothing");
			held->enabled = false;
			return nullptr;
		}

		VIVIUM_LOG(LogSeverity::DEBUG, "Element is released on something {}", hovered->root.base.index);
		// Figure out which area we're holding to figure out the action to take
		GUIProperties const& selectedProperties = properties(hovered->root.base, context);
		// TODO: probably want the extent of the container not position/dimension
		// TODO: we really want to just look at the position and dimension of the root element
		//	but this isn't realistically possible? because we don't have access to the panel
		float bot = selectedProperties.minExtent.y;
		float top = selectedProperties.maxExtent.y;
		float height = top - bot;

		float bot_quarter = bot + height * 0.25f;
		float top_quarter = bot + height * 0.75f;

		// TODO: this reference is a bit dodgy but required
		TreeContainer& hoveredElement = *hovered;
		// Note this is a copy, because otherwise we point to something that doesn't exist anymore
		TreeContainer heldElement = *held;
		heldElement.enabled = true;

		// TODO: the held element (and hovered) is a pointer that has now changed
		// Remove held element from container
		removeChild(container, heldElement, context);
		// Get position of selected
		// TODO: requires us to find the parent
		TreeContainer* parentSelected = findParent(container, hoveredElement, context);

		VIVIUM_ASSERT(parentSelected != nullptr, "Couldn't find parent of selected element");

		uint64_t selectedChildPosition = getChildPosition(parentSelected->root.base, hoveredElement.root.base, context);

		// If in the top 1/4, add above
		if (cursorPosition.y < bot_quarter) {
			// TODO: insert child method
			VIVIUM_LOG(LogSeverity::DEBUG, "Inserting above");
			insertChild(*parentSelected, heldElement, selectedChildPosition, context);
		}
		// If in the bot 1/4, add below
		else if (cursorPosition.y > top_quarter) {
			VIVIUM_LOG(LogSeverity::DEBUG, "Inserting below");
			insertChild(*parentSelected, heldElement, selectedChildPosition + 1, context);
		}
		// If in the middle 1/2, add as child
		else {
			VIVIUM_LOG(LogSeverity::DEBUG, "Inserting into");
			addChild(hoveredElement, heldElement, context);
		}

		return nullptr;
	}

	return held;
}
