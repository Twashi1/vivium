#pragma once

#include "context.h"
#include "../../color.h"

namespace Vivium {
	struct Panel {
		GUIElement* base;
		Color backgroundColor;
		Color borderColor;
		float borderSize;
	};

	struct PanelSpecification {
		GUIElement* parent;
		Color backgroundColor;
		Color borderColor;
		float borderSize; // Border size as percentage
	};

	Panel createPanel(GUI::Visual::Context::Handle guiContext, PanelSpecification specification);
	void dropPanel(Panel& panel, GUI::Visual::Context::Handle guiContext);
	void renderPanels(const std::span<Panel*> panels, Commands::Context::Handle context, GUI::Visual::Context::Handle guiContext, Window::Handle window);
}