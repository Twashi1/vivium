#pragma once

#include "text.h"

namespace Vivium {
	struct Button {
		GUIElement* base;

		Text text;
		TextMetrics metrics;

		Color color;
	};

	void dropButton(Button& button, Engine::Handle engine);
	// TODO: generic render target
	Button submitButton(ResourceManager::Static::Handle manager, GUI::Visual::Context::Handle guiContext, Engine::Handle engine, Window::Handle window);
	void setupButton(Button& button, ResourceManager::Static::Handle manager);
	void setButtonText(Button& button, Engine::Handle engine, Window::Handle window, Commands::Context::Handle context, std::string_view const& text);

	void renderButtons(const std::span<Button*> buttons, Commands::Context::Handle context, GUI::Visual::Context::Handle guiContext, Window::Handle window);
}