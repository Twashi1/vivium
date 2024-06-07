#include "button.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			namespace Button {
				void setup(Button::Handle button, ResourceManager::Static::Handle manager)
				{
					Text::setup(button->text, manager);
				}

				void setText(Button::Handle button, Engine::Handle engine, Window::Handle window, Commands::Context::Handle context, const std::string_view& text)
				{
					// Early exit if no text
					if (text.size() == 0) return;

					button->textMetrics = Text::calculateMetrics(text, button->text->font);

					GUI::Object::update(button, Window::dimensions(window));

					Text::setText(button->text, engine, button->textMetrics, context, text, Text::Alignment::CENTER);
				}
			}
		}
	}
}