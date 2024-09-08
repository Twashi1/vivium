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

	struct GUIElementReference {
		uint64_t index;
	};

	struct GUIElement {
		GUIProperties properties;

		std::vector<GUIElementReference> children;
	};

	struct GUIContext;

	// TODO: versions of these functions for general objects, like Button, Text, etc.
	void updateGUIElement(GUIElementReference const objectHandle, GUIElementReference const parent, F32x2 windowDimensions, GUIContext& guiContext);
	GUIProperties& properties(GUIElementReference const objectHandle, GUIContext& guiContext);
	void addChild(GUIElementReference const parent, std::span<GUIElementReference const> children, GUIContext& guiContext);
}