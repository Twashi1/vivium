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
			Properties(F32x2 dimensions, F32x2 position, PositionType positionType, ScaleType scaleType, Anchor anchorX, Anchor anchorY, Anchor centerX, Anchor centerY);
		};

		bool pointInObject(F32x2 point, Properties const& properties);

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

			void _updateHandle(Handle objectHandle, F32x2 windowDimensions);
			Properties& _properties(Handle object);
			void _addChild(Handle parent, std::span<Handle> child);

			template <typename T>
			concept GUIElement = requires(T element) {
				{ element->base } -> std::same_as<Handle&>;
			} || std::is_same_v<T, Object::Handle>;

			template <GUIElement T>
			Object::Handle _extract(T& element) {
				if constexpr (std::is_same_v<T, Object::Handle>)
					return element;

				return element->base;
			}

			template <GUIElement T>
			void update(T& element, F32x2 windowDimensions) {
				_updateHandle(_extract(element), windowDimensions);
			}

			template <GUIElement T>
			Properties& properties(T& element) {
				return _extract(element)->properties;
			}

			template <GUIElement T>
			Properties const& properties(T const& element) {
				return _extract(element)->properties;
			}

			template <GUIElement T, GUIElement U>
			void addChild(T& element, U& child) {
				_addChild(_extract(element), { _extract(child), 1 });
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