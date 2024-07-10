#include "base.h"

namespace Vivium {
	bool pointInElement(F32x2 point, GUIProperties const& properties) {
		return Math::pointInAABB(point, properties.truePosition, properties.truePosition + properties.trueDimensions);
	}

	void _updateGUIElement(GUIElement* const handle, GUIElement const* const parent, F32x2 windowDimensions) {
		VIVIUM_CHECK_HANDLE_EXISTS(handle);

		// If we have no parent, resort to using window as a pseudo-parent
		F32x2 parentDimensions = parent == VIVIUM_NULL_HANDLE ? windowDimensions : parent->properties.trueDimensions;
		F32x2 parentPosition = parent == VIVIUM_NULL_HANDLE ? F32x2(0.0f) : parent->properties.truePosition;

		F32x2 multiplier = F32x2(0.0f);

		switch (handle->properties.scaleType) {
		case GUIScaleType::FIXED:		multiplier = F32x2(1.0f); break;
		case GUIScaleType::VIEWPORT:	multiplier = windowDimensions; break;
		case GUIScaleType::RELATIVE:	multiplier = parentDimensions; break;
		default: VIVIUM_LOG(Log::FATAL, "Invalid scale type"); break;
		}

		handle->properties.trueDimensions = handle->properties.dimensions * multiplier;
		handle->properties.truePosition = handle->properties.position * multiplier;

		if (handle->properties.positionType == GUIPositionType::RELATIVE) {
			handle->properties.truePosition += parentPosition;

			switch (handle->properties.anchorX) {
			case GUIAnchor::LEFT: break;
			case GUIAnchor::RIGHT:
				handle->properties.truePosition.x += parentDimensions.x; break;
			case GUIAnchor::CENTER:
				handle->properties.truePosition.x += 0.5f * parentDimensions.x; break;
			default:
				VIVIUM_LOG(Log::FATAL, "Invalid anchor for horizontal direction"); break;
			}

			switch (handle->properties.anchorY) {
			case GUIAnchor::BOTTOM: break;
			case GUIAnchor::TOP:
				handle->properties.truePosition.y += parentDimensions.y; break;
			case GUIAnchor::CENTER:
				handle->properties.truePosition.y += 0.5f * parentDimensions.y; break;
			default:
				VIVIUM_LOG(Log::FATAL, "Invalid anchor for vertical direction"); break;
			}

			switch (handle->properties.centerX) {
			case GUIAnchor::LEFT: break;
			case GUIAnchor::RIGHT:
				handle->properties.truePosition.x -= handle->properties.trueDimensions.x;
				break;
			case GUIAnchor::CENTER:
				handle->properties.truePosition.x -= handle->properties.trueDimensions.x * 0.5f;
				break;
			default:
				VIVIUM_LOG(Log::FATAL, "Invalid anchor for horizontal direction"); break;
			}

			switch (handle->properties.centerY) {
			case GUIAnchor::BOTTOM: break;
			case GUIAnchor::TOP:
				handle->properties.truePosition.y -= handle->properties.trueDimensions.y;
				break;
			case GUIAnchor::CENTER:
				handle->properties.truePosition.y -= handle->properties.trueDimensions.y * 0.5f;
				break;
			default:
				VIVIUM_LOG(Log::FATAL, "Invalid anchor for vertical direction"); break;
			}
		}

		for (GUIElement* const child : handle->children)
			_updateGUIElement(child, handle, windowDimensions);
	}
			
	GUIProperties& _properties(GUIElement* const object)
	{
		VIVIUM_CHECK_HANDLE_EXISTS(object);

		return object->properties;
	}
			
	void _addChild(GUIElement* const parent, std::span<GUIElement*> children)
	{
		for (GUIElement* child : children) {
			parent->children.push_back(child);
		}
	}
}