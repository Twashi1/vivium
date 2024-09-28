#pragma once

#include "text.h"

namespace Vivium {
	struct Button {
		GUIElementReference base;

		Text text;
		TextBatch textBatch;

		Color color;
		Color textColor;
	};

	struct ButtonSpecification {
		GUIElementReference parent;

		Color color;
		Color textColor;
	};

	void dropButton(Button& button, Engine& engine, GUIContext& guiContext);
	// TODO: generic render target
	Button submitButton(ResourceManager& manager, GUIContext& guiContext, Engine& engine, Window& window, ButtonSpecification specification);
	void setupButton(Button& button, ResourceManager& manager);
	void setButtonText(Button& button, Engine& engine, Window& window, CommandContext& context, GUIContext& guiContext, std::string_view text);

	void renderButtons(const std::span<Button*> buttons, CommandContext& context, GUIContext& guiContext, Window& window);
}