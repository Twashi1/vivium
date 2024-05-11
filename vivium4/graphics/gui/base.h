#pragma once

#include <span>
#include <vector>

#include "../../math/vec2.h"
#include "../../error/log.h"
#include "../../storage.h"

namespace Vivium {
	namespace GUI {
		enum class PositionType {
			RELATIVE, // Offset to parent
			FIXED	  // Fixed
		};

		enum class ScaleType {
			FIXED,		// Scale in pixels
			VIEWPORT,	// Percentage of viewport dimensions
			RELATIVE	// Percentage of parent dimensions
		};

		enum class Anchor {
			UNDEFINED, // Only valid with PositionType::FIXED
			LEFT,
			CENTER,
			RIGHT,
			TOP,
			BOTTOM
		};

		struct Properties {
			F32x2 dimensions;
			F32x2 position;

			F32x2 truePosition;
			F32x2 trueDimensions;

			PositionType positionType;
			ScaleType scaleType;

			Anchor anchorX, anchorY;

			Properties();
		};

		namespace Object {
			struct Resource;

			typedef Resource* Handle;

			struct Resource {
				Properties properties;

				Handle parent;
				std::vector<Handle> children;
			};

			struct Specification {
				Properties properties;

				Handle parent;
				std::vector<Handle> children;
			};

			void updateHandle(Handle objectHandle, F32x2 windowDimensions);
			Properties& properties(Handle object);

			// TODO: this system is horrendous, change
			template <typename T>
			concept GUIElementPtr = requires(T element) {
				{ element->base } -> std::same_as<Handle&>;
			};

			template <typename T>
			concept GUIElement = requires(T element) {
				{ element.base } -> std::same_as<Handle&>;
			};

			template <GUIElementPtr T>
			void update(T element, F32x2 windowDimensions) {
				updateHandle(element->base, windowDimensions);
			}

			template <GUIElement T>
			void update(T element, F32x2 windowDimensions) {
				updateHandle(element.base, windowDimensions);
			}

			template <Allocator::AllocatorType AllocatorType>
			Handle create(AllocatorType allocator, Specification specification) {
				Handle handle = Allocator::allocateResource<Resource>(allocator);

				handle->properties = specification.properties;
				handle->children = specification.children;

				return handle;
			}

			template <Allocator::AllocatorType AllocatorType>
			void drop(AllocatorType allocator, Handle handle) {
				Allocator::dropResource(allocator, handle);
			}
		}
	}
}