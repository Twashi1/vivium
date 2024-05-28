#pragma once

#include "text.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			namespace Button {
				// TODO: use text->base, set as child class, do everything for that etc.
				struct Resource {
					Object::Handle base;

					Text::Handle text;
					Text::Metrics textMetrics; // Stored for rendering
					Color color;

					Buffer::Handle stagingVertex;
					Buffer::Handle stagingIndex;

					Buffer::Handle deviceVertex;
					Buffer::Handle deviceIndex;

					Buffer::Handle uniformBuffer;

					DescriptorSet::Handle descriptorSet;
				};

				typedef Resource* Handle;
				typedef Resource* PromisedHandle;

				template <Allocator::AllocatorType AllocatorType>
				void drop(AllocatorType* allocator, Handle handle, Engine::Handle engine) {
					VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);

					GUI::Object::drop(allocator, handle->base);

					Text::drop(allocator, handle->text, engine);

					Buffer::drop(VIVIUM_NULL_ALLOCATOR, handle->stagingVertex, engine);
					Buffer::drop(VIVIUM_NULL_ALLOCATOR, handle->stagingIndex, engine);
					Buffer::drop(VIVIUM_NULL_ALLOCATOR, handle->deviceVertex, engine);
					Buffer::drop(VIVIUM_NULL_ALLOCATOR, handle->deviceIndex, engine);
					Buffer::drop(VIVIUM_NULL_ALLOCATOR, handle->uniformBuffer, engine);

					DescriptorSet::drop(allocator, handle->descriptorSet);

					Allocator::dropResource(allocator, handle);
				}

				template <Allocator::AllocatorType AllocatorType>
				PromisedHandle submit(AllocatorType* allocator, ResourceManager::Static::Handle manager, Context::Handle guiContext, Engine::Handle engine, Window::Handle window)
				{
					PromisedHandle button = Allocator::allocateResource<Resource, AllocatorType>(allocator);

					button->base = GUI::Object::create(allocator, GUI::Object::Specification{});
					button->color = Color::Gray;

					std::vector<Buffer::Handle> hostBuffers = ResourceManager::Static::submit(manager, MemoryType::STAGING, std::vector<Buffer::Specification>({
						Buffer::Specification(4 * sizeof(F32x2), Buffer::Usage::STAGING),
						Buffer::Specification(6 * sizeof(uint16_t), Buffer::Usage::STAGING),
						Buffer::Specification(sizeof(F32x2) * 2 + sizeof(Color), Buffer::Usage::UNIFORM)
						}));

					button->stagingVertex = hostBuffers[0];
					button->stagingIndex = hostBuffers[1];
					button->uniformBuffer = hostBuffers[2];

					std::vector<Buffer::Handle> deviceBuffers = ResourceManager::Static::submit(manager, MemoryType::DEVICE,
						std::vector<Buffer::Specification>({
							Buffer::Specification(4 * sizeof(F32x2), Buffer::Usage::VERTEX),
							Buffer::Specification(6 * sizeof(uint16_t), Buffer::Usage::INDEX)
							}));

					button->deviceVertex = deviceBuffers[0];
					button->deviceIndex = deviceBuffers[1];

					std::vector<DescriptorSet::Handle> descriptorSets = ResourceManager::Static::submit(
						manager, std::vector<DescriptorSet::Specification>({
							DescriptorSet::Specification(
								guiContext->button.descriptorLayout, std::vector<Uniform::Data>({
									Uniform::Data::fromBuffer(button->uniformBuffer, sizeof(F32x2) * 2 + sizeof(Color), 0)
								})
							)
							})
					);

					button->descriptorSet = descriptorSets[0];

					// TODO: maximum text length should be parameter
					button->text = Text::submit(allocator, manager, engine, guiContext, Text::Specification(64, Font::Font::fromDistanceFieldFile("testGame/res/fonts/consola.sdf")));
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

				void setup(Button::Handle button, Commands::Context::Handle context, Engine::Handle engine);
				void render(Button::Handle button, Commands::Context::Handle context, Context::Handle textContext, Window::Handle window, Math::Perspective perspective);

				void setText(Button::Handle button, Engine::Handle engine, Window::Handle window, Commands::Context::Handle context, const std::string_view& text);
			}
		}
	}
}