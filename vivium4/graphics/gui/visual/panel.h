#pragma once

#include "context.h"
#include "../../color.h"

namespace Vivium {
	struct Panel {
		GUIElementReference base;
		Color backgroundColor;
		Color borderColor;
		float borderSize;
	};

	struct PanelSpecification {
		GUIElementReference parent;
		Color backgroundColor;
		Color borderColor;
		float borderSize; // Border size as percentage
	};

	Panel createPanel(GUIContext& guiContext, PanelSpecification specification);
	void submitPanels(std::span<Panel*> const panels, GUIContext& guiContext);
	void renderPanels(CommandContext& context, GUIContext& guiContext, Window& window);
}