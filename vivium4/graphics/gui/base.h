#pragma once

#include <span>
#include <vector>

#include "../../math/vec2.h"
#include "../../math/aabb.h"
#include "../../error/log.h"
#include "../../storage.h"

namespace Vivium {
	enum class GUIPositionType {
		RELATIVE, // Offset to parent
		FIXED	  // Fixed
	};

	// TODO: VIEWPORT_HEIGHT and VIEWPORT_WIDTH
	enum class GUIScaleType {
		FIXED,		// Scale in pixels
		VIEWPORT,	// FRACTION of viewport dimensions
		RELATIVE	// FRACTION of parent dimensions
	};

	enum class GUIAnchor {
		UNDEFINED, // Only valid with PositionType::FIXED
		LEFT,
		CENTER,
		RIGHT,
		TOP,
		BOTTOM
	};

	struct GUIProperties {
		F32x2 dimensions = F32x2(1.0f);
		F32x2 position = F32x2(0.0f);

		GUIPositionType positionType = GUIPositionType::RELATIVE;
		GUIScaleType scaleType = GUIScaleType::RELATIVE;

		GUIAnchor anchorX = GUIAnchor::CENTER;
		GUIAnchor anchorY = GUIAnchor::CENTER;
		// TODO: different type? or generalise "Anchor"
		GUIAnchor centerX = GUIAnchor::CENTER;
		GUIAnchor centerY = GUIAnchor::CENTER;

		F32x2 truePosition = F32x2(0.0f);
		F32x2 trueDimensions = F32x2(0.0f);
	};

	bool pointInElement(F32x2 point, GUIProperties const& properties);

	struct GUIElement {
		GUIProperties properties;

		std::vector<GUIElement*> children;
	};

	void _updateGUIElement(GUIElement* const objectHandle, GUIElement const* const parent, F32x2 windowDimensions);
	GUIProperties& _properties(GUIElement* const objectHandle);
	void _addChild(GUIElement* const parent, std::span<GUIElement*> child);

	template <typename T>
	concept GenericGUIElement = requires(T element) {
		{ element.base } -> std::convertible_to<GUIElement*>;
	} || std::is_same_v<T, GUIElement*>;

	template <GenericGUIElement T>
	GUIElement* _extract(T& element) {
		if constexpr (std::is_same_v<T, GUIElement*>)
			return element;

		return element.base;
	}

	template <GenericGUIElement T>
	void update(T& element, F32x2 windowDimensions) {
		_updateGUIElement(_extract(element), nullptr, windowDimensions);
	}

	template <GenericGUIElement T>
	GUIProperties* properties(T& element) {
		return &_extract(element)->properties;
	}

	template <GenericGUIElement T, GenericGUIElement U>
	void addChild(T& element, U& child) {
		_addChild(_extract(element), { _extract(child), 1 });
	}
}