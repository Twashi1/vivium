#pragma once

#include "context.h"
#include "../../color.h"

namespace Vivium {
	struct Panel {
		GUIElement* base;

		Color backgroundColor;
	};

	struct PanelSpecification {
		Color backgroundColor;
	};

	Panel createPanel(GUI::Visual::Context::Handle guiContext, PanelSpecification const& specification);
	void renderPanels(const std::span<Panel*> panels, Commands::Context::Handle context, GUI::Visual::Context::Handle guiContext, Window::Handle window);
}