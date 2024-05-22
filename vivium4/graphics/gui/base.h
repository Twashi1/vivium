#pragma once

#include <span>
#include <vector>

#include "../../math/vec2.h"
#include "../../math/aabb.h"
#include "../../error/log.h"
#include "../../storage.h"

namespace Vivium {
	namespace GUI {
		enum class PositionType {
			RELATIVE, // Offset to parent
			FIXED	  // Fixed
		};

		// TODO: VIEWPORT_HEIGHT and VIEWPORT_WIDTH
		enum class ScaleType {
			FIXED,		// Scale in pixels
			VIEWPORT,	// FRACTION of viewport dimensions
			RELATIVE	// FRACTION of parent dimensions
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
			// TODO: different type? or generalise "Anchor"
			Anchor centerX, centerY;

			Properties();
		};

		bool pointInObject(F32x2 point, Properties properties);

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

				Specification();
			};

			// TODO: make clearly private
			void updateHandle(Handle objectHandle, F32x2 windowDimensions);
			Properties& properties(Handle object);
			void addChild(Handle parent, std::span<Handle> child);

			template <typename T>
			concept GUIElement = requires(T element) {
				{ element->base } -> std::same_as<Handle&>;
			};

			template <GUIElement T>
			void update(T element, F32x2 windowDimensions) {
				updateHandle(element->base, windowDimensions);
			}

			template <GUIElement T>
			Properties& properties(T element) {
				return properties(element->base);
			}

			template <Allocator::AllocatorType AllocatorType>
			Handle create(AllocatorType* allocator, Specification specification) {
				Handle handle = Allocator::allocateResource<Resource>(allocator);

				handle->properties = specification.properties;
				handle->children = specification.children;

				return handle;
			}

			template <Allocator::AllocatorType AllocatorType>
			void drop(AllocatorType* allocator, Handle handle) {
				Allocator::dropResource(allocator, handle);
			}
		}
	}
}