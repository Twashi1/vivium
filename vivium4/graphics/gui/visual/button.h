#pragma once

#include "text.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			namespace Button {
				struct Resource {
					Object::Handle base;

					Text::Handle text;
					Text::Metrics textMetrics; // Stored for rendering
					Color color;
				};

				typedef Resource* Handle;
				typedef Resource* PromisedHandle;

				template <Storage::StorageType StorageType>
				void drop(StorageType* allocator, Handle handle, Engine::Handle engine) {
					GUI::Object::drop(allocator, handle->base);

					Text::drop(allocator, handle->text, engine);

					Storage::dropResource(allocator, handle);
				}

				template <Storage::StorageType StorageType>
				PromisedHandle submit(StorageType* allocator, ResourceManager::Static::Handle manager, Context::Handle guiContext, Engine::Handle engine, Window::Handle window)
				{
					PromisedHandle button = Storage::allocateResource<Resource, StorageType>(allocator);

					button->base = GUI::Object::create(allocator, GUI::Object::Specification{});
					button->color = Color::White;

					// TODO: maximum text length should be parameter
					button->text = Text::submit(allocator, manager, engine, guiContext, Text::Specification(64, Font::Font::fromDistanceFieldFile("res/fonts/consola.sdf")));
					// TODO: use different method
					_addChild(button->base, { &button->text->base, 1 });
					Properties& textProperties = GUI::Object::properties(button->text);
					textProperties.dimensions = F32x2(0.9f);
					textProperties.position = F32x2(0.0f);
					textProperties.scaleType = GUI::ScaleType::RELATIVE;
					textProperties.positionType = GUI::PositionType::RELATIVE;
					textProperties.anchorX = Anchor::CENTER;
					textProperties.anchorY = Anchor::CENTER;
					textProperties.centerX = Anchor::LEFT;
					textProperties.centerY = Anchor::BOTTOM;

					return button;
				}

				void setup(Button::Handle button, ResourceManager::Static::Handle manager);

				void setText(Button::Handle button, Engine::Handle engine, Window::Handle window, Commands::Context::Handle context, const std::string_view& text);
			}
		}
	}
}