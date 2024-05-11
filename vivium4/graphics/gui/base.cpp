#include "base.h"

namespace Vivium {
	namespace GUI {
		Properties::Properties()
			: positionType(PositionType::RELATIVE), scaleType(ScaleType::VIEWPORT), anchorX(Anchor::CENTER), anchorY(Anchor::CENTER), truePosition(0.0f), trueDimensions(0.0f)
		{}

		namespace Object {
			void updateHandle(Handle handle, F32x2 windowDimensions) {
				VIVIUM_CHECK_HANDLE_EXISTS(handle);

				// If we have no parent, resort to using window as a pseudo-parent
				F32x2 parentDimensions = handle->parent == VK_NULL_HANDLE ? windowDimensions : handle->parent->properties.trueDimensions;
				F32x2 parentPosition = handle->parent == VK_NULL_HANDLE ? F32x2(0.0f) : handle->parent->properties.truePosition;

				F32x2 multiplier = F32x2(0.0f);

				switch (handle->properties.scaleType) {
				case ScaleType::FIXED:		multiplier = F32x2(1.0f); break;
				case ScaleType::VIEWPORT:	multiplier = windowDimensions; break;
				case ScaleType::RELATIVE:	multiplier = parentDimensions; break;
				default: VIVIUM_LOG(Log::FATAL, "Invalid scale type"); break;
				}

				handle->properties.trueDimensions = handle->properties.dimensions * multiplier;
				handle->properties.truePosition = handle->properties.position * multiplier;

				if (handle->properties.positionType == PositionType::RELATIVE) {
					handle->properties.truePosition += parentPosition;

					switch (handle->properties.anchorX) {
					case Anchor::LEFT: break;
					case Anchor::RIGHT:
						handle->properties.truePosition.x += parentDimensions.x - handle->properties.trueDimensions.x; break;
					case Anchor::CENTER:
						handle->properties.truePosition.x += 0.5f * (parentDimensions.x - handle->properties.trueDimensions.x); break;
					default:
						VIVIUM_LOG(Log::FATAL, "Invalid anchor for horizontal direction"); break;
					}

					switch (handle->properties.anchorY) {
					case Anchor::BOTTOM: break;
					case Anchor::TOP:
						handle->properties.truePosition.y += parentDimensions.y - handle->properties.trueDimensions.y; break;
					case Anchor::CENTER:
						handle->properties.truePosition.y += 0.5f * (parentDimensions.y - handle->properties.trueDimensions.y); break;
					default:
						VIVIUM_LOG(Log::FATAL, "Invalid anchor for vertical direction"); break;
					}
				}

				for (Handle child : handle->children)
					updateHandle(handle, windowDimensions);
			}
			
			Properties& properties(GUI::Object::Handle object)
			{
				VIVIUM_CHECK_HANDLE_EXISTS(object);

				return object->properties;
			}
		}
	}
}