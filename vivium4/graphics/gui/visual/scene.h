#pragma once

#include "button.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			struct Scene {
				std::vector<Button::Handle> buttons;
				// TODO
				// std::vector<Text::Handle> texts;
			};

			void renderScene(Scene& scene, Commands::Context::Handle context, GUI::Visual::Context::Handle guiContext, Window::Handle window);
		}
	}
}