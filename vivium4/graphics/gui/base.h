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
	enum class GUIUnits {
		PIXELS,		// Scale in pixels
		VIEWPORT,	// Fraction of viewport dimensions
		RELATIVE	// Fraction of parent dimensions
	};

	enum class GUIAnchor {
		NONE, // Only valid with a fixed position type
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
		GUIUnits unitsType = GUIUnits::RELATIVE;

		GUIAnchor anchorX = GUIAnchor::CENTER;
		GUIAnchor anchorY = GUIAnchor::CENTER;
		// TODO: different type? or generalise "Anchor"
		GUIAnchor centerX = GUIAnchor::CENTER;
		GUIAnchor centerY = GUIAnchor::CENTER;

		// Bottom left corner in px
		F32x2 truePosition = F32x2(0.0f);
		// Dimensions of each axis in px
		F32x2 trueDimensions = F32x2(0.0f);
	};

	bool pointInElement(F32x2 point, GUIProperties const& properties);

	enum GUIElementType {
		DEFAULT,
		CONTAINER_VERTICAL,
		CONTAINER_HORIZONTAL
	};

	struct GUIElementReference {
		uint64_t index;
		GUIElementType type;
	};

	bool operator==(GUIElementReference const& a, GUIElementReference const& b);

	struct GUIElement {
		GUIProperties properties;

		std::vector<GUIElementReference> children;
	};

	struct GUIContext;

	// TODO: versions of these functions for general objects, like Button, Text, etc.
	void updateGUIElement(GUIElementReference const objectHandle, GUIElementReference const parent, F32x2 windowDimensions, GUIContext& guiContext);
	GUIProperties& properties(GUIElementReference const objectHandle, GUIContext& guiContext);
	void addChild(GUIElementReference const parent, std::span<GUIElementReference const> children, GUIContext& guiContext);
	void removeChild(GUIElementReference const parent, std::span<GUIElementReference const> children, GUIContext& guiContext);
	// TODO: make use of this method
	GUIElement const& _getGUIElement(GUIElementReference const reference, GUIContext const& guiContext);

	template <typename T>
	concept GUIGeneric = requires (T object) {
		{object.base} -> std::same_as<GUIElementReference&>;
	} || std::is_same_v<T, GUIElementReference>;

	// Should only realistically be updating head of tree
	void updateGUI(F32x2 windowDimensions, GUIContext& guiContext);

	template <GUIGeneric T>
	GUIElementReference _extractGUIReference(T const& object) {
		if constexpr (std::is_same_v<T, GUIElementReference>) { return object; }
		else { return object.base; }
	}

	template <GUIGeneric T>
	GUIProperties& properties(T const& object, GUIContext& guiContext) {
		return properties(_extractGUIReference(object), guiContext);
	}

	template <GUIGeneric U, GUIGeneric V>
	void addChild(U const& object, V const& child, GUIContext& guiContext) {
		// TODO: maybe don't need this copy to get around const
		GUIElementReference copy = _extractGUIReference(child);
		addChild(_extractGUIReference(object), { &copy, 1 }, guiContext);
	}

	template <GUIGeneric U, GUIGeneric V>
	void removeChild(U const& object, V const& child, GUIContext& guiContext) {
		GUIElementReference copy = _extractGUIReference(child);
		removeChild(_extractGUIReference(object), { &copy, 1 }, guiContext);
	}
}