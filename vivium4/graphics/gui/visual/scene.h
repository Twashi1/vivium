#pragma once

#include "button.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			void renderButtons(const std::span<const Button::Handle> buttons, Commands::Context::Handle context, GUI::Visual::Context::Handle guiContext, Window::Handle window);
		}
	}
}