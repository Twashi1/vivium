#include "base.h"
#include "visual/context.h"

namespace Vivium {
	bool pointInElement(F32x2 point, GUIProperties const& properties) {
		return Math::pointInAABB(point, properties.truePosition, properties.truePosition + properties.trueDimensions);
	}

	void updateGUIElement(GUIElementReference const element, GUIElementReference const parent, F32x2 windowDimensions, GUIContext& context) {
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

		for (GUIElementReference child : object.children)
		{
			updateGUIElement(child, element, windowDimensions, context);
		}
	}
			
	GUIProperties& properties(GUIElementReference const objectHandle, GUIContext& guiContext)
	{
		return guiContext.guiElements[objectHandle.index].properties;
	}

	void addChild(GUIElementReference const parent, std::span<GUIElementReference const> children, GUIContext& guiContext)
	{
		GUIElement& parentObject = guiContext.guiElements[parent.index];

		parentObject.children.insert(parentObject.children.end(), children.begin(), children.end());
	}
	
	void updateGUI(F32x2 windowDimensions, GUIContext& guiContext)
	{
		updateGUIElement(guiContext.defaultParent, guiContext.defaultParent, windowDimensions, guiContext);
	}
}