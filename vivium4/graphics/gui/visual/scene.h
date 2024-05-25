#pragma once

#include "button.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			struct Scene {
				std::vector<Button::Handle> buttons;
				std::vector<Text::Handle> texts;


			};
		}
	}
}