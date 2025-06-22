#include "tree_container.h"

bool operator==(TreeContainer const& a, TreeContainer const& b)
{
	return a.root.base == b.root.base;
}

TreeContainer createTreeContainer(GUIContext& guiContext, GUIElementReference parent) {
	TreeContainer tree;

	tree.root = createContainer(guiContext, ContainerSpecification(parent, ContainerOrdering::VERTICAL));

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
		std::remove(container.children.begin(), container.children.end(), child);
		removeChild(container.root.base, { &child.root.base, 1 }, guiContext);
	}
	else {
		for (TreeContainer& nextContainer : container.children) {
			removeChild(nextContainer, child, guiContext);
		}
	}
}

TreeContainer* getContainer(F32x2 position, TreeContainer& container, GUIContext& guiContext) {
	if (container.enabled && pointInElement(position, properties(container.root, guiContext))) {
		return &container;
	}

	for (TreeContainer& child : container.children) {
		TreeContainer* result = getContainer(position, child, guiContext);

		if (result != nullptr) {
			return result;
		}
	}

	return nullptr;
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

TreeContainer* updateTreeContainer(F32x2 cursorPosition, TreeContainer& container, TreeContainer* held, GUIContext& context)
{
	// TODO: should not be able to pick up the root
	// TODO: rename to hovered
	TreeContainer* selectedElement = getContainer(cursorPosition, container, context);

	// If we're not holding anything, and we pressed left, then select this element
	if (held == nullptr && Input::get(Input::BTN_1).state == Input::DOWN) {
		selectedElement->enabled = false;
		return selectedElement;
	}

	// If we've not selected anything and we released
	if (selectedElement == nullptr && Input::get(Input::BTN_1).state == Input::UP) {
		// Re-enable the held element
		if (held != nullptr) {
			held->enabled = true;
		}
		// Return nullptr to indicate we've stopped holding anything

		return nullptr;
	}
	// If we're holding something, and we've released on something selected
	if (selectedElement != nullptr && Input::get(Input::BTN_1).state == Input::UP && held != nullptr) {
		// Figure out which area we're holding to figure out the action to take
		GUIProperties selectedProperties = properties(selectedElement->root.base, context);
		float bot = selectedProperties.truePosition.y;
		float height = selectedProperties.trueDimensions.y;
		float top = bot + height;

		float bot_quarter = bot + height * 0.25f;
		float top_quarter = bot + height * 0.75f;

		// Remove held element from container
		removeChild(container, *held, context);
		// Get position of selected
		// TODO: requires us to find the parent
		TreeContainer* parentSelected = findParent(container, *selectedElement, context);

		VIVIUM_ASSERT(parentSelected != nullptr, "Couldn't find parent of selected element");

		uint64_t selectedChildPosition = getChildPosition(parentSelected->root.base, selectedElement->root.base, context);

		// If in the top 1/4, add above
		if (cursorPosition.y < bot_quarter) {
			// TODO: insert child method
			insertChild(*parentSelected, *held, selectedChildPosition, context);
		}
		// If in the bot 1/4, add below
		else if (cursorPosition.y > top_quarter) {
			insertChild(*parentSelected, *held, selectedChildPosition + 1, context);
		}
		// If in the middle 1/2, add as child
		else {
			addChild(*selectedElement, *held, context);
		}
	}

	return nullptr;
}
