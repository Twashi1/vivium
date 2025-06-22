#include "base.h"
#include "visual/context.h"
#include "visual/container.h"

namespace Vivium {
	bool pointInElement(F32x2 point, GUIProperties const& properties)
	{
		return pointInAABB(point, properties.truePosition, properties.truePosition + properties.trueDimensions);
	}

	bool operator==(GUIElementReference const& a, GUIElementReference const& b)
	{
		return a.index == b.index;
	}

	void _updateContainer(GUIElementReference reference, _ContainerUpdateData containerData, F32x2 windowDimensions, GUIContext& context)
	{
		F32x2 totalOffset = F32x2(0.0f);

		// TODO: some method to grab reference to object
		GUIElement& element = context.guiElements[reference.index];

		for (GUIElementReference child : element.children)
		{
			updateGUIElement(child, reference, windowDimensions, context);

			// TODO: use min/max extent calculations instead
			F32x2 childOffset = properties(reference, context).trueDimensions + properties(reference, context).truePosition - properties(child, context).truePosition;
			F32x2 newOffset = properties(child, context).maxExtent - properties(child, context).minExtent;

			if (containerData.ordering == ContainerOrdering::VERTICAL) { newOffset.x = 0.0f; }
			if (containerData.ordering == ContainerOrdering::HORIZONTAL) { newOffset.y = 0.0f; }

			properties(reference, context).truePosition -= newOffset;
			totalOffset += newOffset;
		}

		properties(reference, context).truePosition += totalOffset;
	}

	void updateGUIElement(GUIElementReference const element, GUIElementReference const parent, F32x2 windowDimensions, GUIContext& context)
	{
		// If we have no parent, resort to using window as a pseudo-parent
		F32x2 parentDimensions = parent.index == NULL ? windowDimensions : context.guiElements[parent.index].properties.trueDimensions;
		F32x2 parentPosition = parent.index == NULL ? F32x2(0.0f) : context.guiElements[parent.index].properties.truePosition;

		F32x2 multiplier = F32x2(0.0f);

		GUIElement& object = context.guiElements[element.index];

		switch (object.properties.unitsType) {
		case GUIUnits::PIXELS:		multiplier = F32x2(1.0f); break;
		case GUIUnits::VIEWPORT:	multiplier = windowDimensions; break;
		case GUIUnits::RELATIVE:	multiplier = parentDimensions; break;
		default: VIVIUM_LOG(LogSeverity::FATAL, "Invalid scale type"); break;
		}

		object.properties.trueDimensions = object.properties.dimensions * multiplier;
		object.properties.truePosition = object.properties.position * multiplier;
		object.properties.minExtent = object.properties.truePosition;
		object.properties.maxExtent = object.properties.truePosition + object.properties.trueDimensions;

		if (object.properties.positionType == GUIPositionType::RELATIVE) {
			object.properties.truePosition += parentPosition;

			switch (object.properties.anchorX) {
			case GUIAnchor::LEFT: break;
			case GUIAnchor::RIGHT:
				object.properties.truePosition.x += parentDimensions.x; break;
			case GUIAnchor::CENTER:
				object.properties.truePosition.x += 0.5f * parentDimensions.x; break;
			default:
				VIVIUM_LOG(LogSeverity::FATAL, "Invalid anchor for horizontal direction"); break;
			}

			switch (object.properties.anchorY) {
			case GUIAnchor::BOTTOM: break;
			case GUIAnchor::TOP:
				object.properties.truePosition.y += parentDimensions.y; break;
			case GUIAnchor::CENTER:
				object.properties.truePosition.y += 0.5f * parentDimensions.y; break;
			default:
				VIVIUM_LOG(LogSeverity::FATAL, "Invalid anchor for vertical direction"); break;
			}

			switch (object.properties.centerX) {
			case GUIAnchor::LEFT: break;
			case GUIAnchor::RIGHT:
				object.properties.truePosition.x -= object.properties.trueDimensions.x;
				break;
			case GUIAnchor::CENTER:
				object.properties.truePosition.x -= object.properties.trueDimensions.x * 0.5f;
				break;
			default:
				VIVIUM_LOG(LogSeverity::FATAL, "Invalid anchor for horizontal direction"); break;
			}

			switch (object.properties.centerY) {
			case GUIAnchor::BOTTOM: break;
			case GUIAnchor::TOP:
				object.properties.truePosition.y -= object.properties.trueDimensions.y;
				break;
			case GUIAnchor::CENTER:
				object.properties.truePosition.y -= object.properties.trueDimensions.y * 0.5f;
				break;
			default:
				VIVIUM_LOG(LogSeverity::FATAL, "Invalid anchor for vertical direction"); break;
			}
		}

		// Look for any required special treatment
		switch (object.type) {
		case GUIElementType::CARDINAL_CONTAINER: return _updateContainer(element, object.data.container, windowDimensions, context);
		default: break;
		}

		// We only update children if its not some special container
		for (GUIElementReference child : object.children)
		{
			updateGUIElement(child, element, windowDimensions, context);
			object.properties.minExtent.x = std::min(object.properties.minExtent.x, properties(child, context).minExtent.x);
			object.properties.minExtent.y = std::min(object.properties.minExtent.y, properties(child, context).minExtent.y);
			object.properties.maxExtent.x = std::max(object.properties.maxExtent.x, properties(child, context).maxExtent.x);
			object.properties.maxExtent.y = std::max(object.properties.maxExtent.y, properties(child, context).maxExtent.y);
		}
	}
			
	GUIProperties& properties(GUIElementReference const objectHandle, GUIContext& guiContext)
	{
		return guiContext.guiElements[objectHandle.index].properties;
	}

	uint64_t getChildPosition(GUIElementReference const parent, GUIElementReference const child, GUIContext& guiContext)
	{
		GUIElement& parentObject = guiContext.guiElements[parent.index];

		return std::distance(parentObject.children.begin(), std::find(parentObject.children.begin(), parentObject.children.end(), child));
	}

	void insertChild(GUIElementReference const parent, std::span<GUIElementReference const> children, uint64_t position, GUIContext& guiContext)
	{
		GUIElement& parentObject = guiContext.guiElements[parent.index];

		parentObject.children.insert(parentObject.children.begin() + position, children.begin(), children.end());
	}

	void addChild(GUIElementReference const parent, std::span<GUIElementReference const> children, GUIContext& guiContext)
	{
		GUIElement& parentObject = guiContext.guiElements[parent.index];

		parentObject.children.insert(parentObject.children.end(), children.begin(), children.end());
	}

	// TODO: O(n^2) algorithm
	void removeChild(GUIElementReference const parent, std::span<GUIElementReference const> children, GUIContext& guiContext)
	{
		GUIElement& parentObject = guiContext.guiElements[parent.index];

		for (GUIElementReference const reference : children) {
			parentObject.children.erase(std::find(parentObject.children.begin(), parentObject.children.end(), reference));
		}
	}

	GUIElement const& _getGUIElement(GUIElementReference const reference, GUIContext const& guiContext)
	{
		return guiContext.guiElements[reference.index];
	}
	
	void updateGUI(F32x2 windowDimensions, GUIContext& guiContext)
	{
		updateGUIElement(guiContext.defaultParent, guiContext.defaultParent, windowDimensions, guiContext);
	}
}