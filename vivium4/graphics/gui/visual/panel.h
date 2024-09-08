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
	void dropPanel(Panel& panel, GUIContext& guiContext);
	void renderPanels(const std::span<Panel*> panels, Commands::Context::Handle context, GUIContext& guiContext, Window::Handle window);
}