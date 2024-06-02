#include "base.h"

namespace Vivium {
	namespace GUI {
		Properties::Properties()
			: positionType(PositionType::RELATIVE), scaleType(ScaleType::VIEWPORT), anchorX(Anchor::CENTER), anchorY(Anchor::CENTER), centerX(Anchor::CENTER), centerY(Anchor::CENTER), truePosition(0.0f), trueDimensions(0.0f)
		{}

		Properties::Properties(F32x2 dimensions, F32x2 position, PositionType positionType, ScaleType scaleType, Anchor anchorX, Anchor anchorY, Anchor centerX, Anchor centerY)
			: dimensions(dimensions), position(position), positionType(positionType), scaleType(scaleType), anchorX(anchorX), anchorY(anchorY), centerX(centerX), centerY(centerY),
			trueDimensions(F32x2(0.0f)), truePosition(F32x2(0.0f))
		{}

		bool pointInObject(F32x2 point, Properties const& properties) {
			return Math::pointInAABB(point, properties.truePosition, properties.truePosition + properties.trueDimensions);
		}

		namespace Object {
			Specification::Specification()
				: parent(VIVIUM_NULL_HANDLE)
			{}

			void _updateHandle(Handle handle, F32x2 windowDimensions) {
				VIVIUM_CHECK_HANDLE_EXISTS(handle);

				// If we have no parent, resort to using window as a pseudo-parent
				F32x2 parentDimensions = handle->parent == VIVIUM_NULL_HANDLE ? windowDimensions : handle->parent->properties.trueDimensions;
				F32x2 parentPosition = handle->parent == VIVIUM_NULL_HANDLE ? F32x2(0.0f) : handle->parent->properties.truePosition;

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
						handle->properties.truePosition.x += parentDimensions.x; break;
					case Anchor::CENTER:
						handle->properties.truePosition.x += 0.5f * parentDimensions.x; break;
					default:
						VIVIUM_LOG(Log::FATAL, "Invalid anchor for horizontal direction"); break;
					}

					switch (handle->properties.anchorY) {
					case Anchor::BOTTOM: break;
					case Anchor::TOP:
						handle->properties.truePosition.y += parentDimensions.y; break;
					case Anchor::CENTER:
						handle->properties.truePosition.y += 0.5f * parentDimensions.y; break;
					default:
						VIVIUM_LOG(Log::FATAL, "Invalid anchor for vertical direction"); break;
					}

					switch (handle->properties.centerX) {
					case Anchor::LEFT: break;
					case Anchor::RIGHT:
						handle->properties.truePosition.x -= handle->properties.trueDimensions.x;
						break;
					case Anchor::CENTER:
						handle->properties.truePosition.x -= handle->properties.trueDimensions.x * 0.5f;
						break;
					default:
						VIVIUM_LOG(Log::FATAL, "Invalid anchor for horizontal direction"); break;
					}

					switch (handle->properties.centerY) {
					case Anchor::BOTTOM: break;
					case Anchor::TOP:
						handle->properties.truePosition.y -= handle->properties.trueDimensions.y;
						break;
					case Anchor::CENTER:
						handle->properties.truePosition.y -= handle->properties.trueDimensions.y * 0.5f;
						break;
					default:
						VIVIUM_LOG(Log::FATAL, "Invalid anchor for vertical direction"); break;
					}
				}

				for (Handle child : handle->children)
					_updateHandle(child, windowDimensions);
			}
			
			Properties& _properties(GUI::Object::Handle object)
			{
				VIVIUM_CHECK_HANDLE_EXISTS(object);

				return object->properties;
			}
			
			void _addChild(Handle parent, std::span<Handle> children)
			{
				for (Handle child : children) {
					parent->children.push_back(child);

					child->parent = parent;
				}
			}
		}
	}
}