#pragma once

#include "../base.h"
#include "../text.h"
#include "../../color.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			namespace Button {
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

					Shader::Handle fragmentShader;
					Shader::Handle vertexShader;

					DescriptorSet::Handle descriptorSet;

					DescriptorLayout::Handle descriptorLayout;
					Pipeline::Handle pipeline;
				};

				typedef Resource* Handle;
				typedef Resource* PromisedHandle;

				template <Allocator::AllocatorType AllocatorType>
				void drop(AllocatorType allocator, Handle handle, Engine::Handle engine) {
					VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);

					GUI::Object::drop(allocator, handle->base);

					Text::drop(allocator, handle->text, engine);

					Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, handle->stagingVertex, engine);
					Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, handle->stagingIndex, engine);
					Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, handle->deviceVertex, engine);
					Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, handle->deviceIndex, engine);
					Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, handle->uniformBuffer, engine);

					Shader::drop(allocator, handle->fragmentShader, engine);
					Shader::drop(allocator, handle->vertexShader, engine);

					DescriptorLayout::drop(allocator, handle->descriptorLayout, engine);
					DescriptorSet::drop(allocator, handle->descriptorSet);
					Pipeline::drop(VIVIUM_RESOURCE_ALLOCATED, handle->pipeline, engine);

					Allocator::dropResource(allocator, handle);
				}

				template <Allocator::AllocatorType AllocatorType>
				PromisedHandle submit(AllocatorType allocator, ResourceManager::Static::Handle manager, Engine::Handle engine)
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

					button->descriptorLayout = DescriptorLayout::create(allocator, engine, DescriptorLayout::Specification(
						std::vector<Uniform::Binding>({ Uniform::Binding(Shader::Stage::FRAGMENT | Shader::Stage::VERTEX, 0, Uniform::Type::UNIFORM_BUFFER) })
					));

					std::vector<DescriptorSet::Handle> descriptorSets = ResourceManager::Static::submit(
						manager, std::vector<DescriptorSet::Specification>({
							DescriptorSet::Specification(
								button->descriptorLayout, std::vector<Uniform::Data>({
									Uniform::Data::fromBuffer(button->uniformBuffer, sizeof(F32x2) * 2 + sizeof(Color), 0)
								})
							)
							})
					);

					button->descriptorSet = descriptorSets[0];

					Shader::Specification fragShaderSpec = Shader::compile(Shader::Stage::FRAGMENT, "testGame/res/button.frag", "testGame/res/button_frag.spv");
					Shader::Specification vertShaderSpec = Shader::compile(Shader::Stage::VERTEX, "testGame/res/button.vert", "testGame/res/button_vert.spv");

					button->fragmentShader = Shader::create(allocator, engine, fragShaderSpec);
					button->vertexShader = Shader::create(allocator, engine, vertShaderSpec);

					// TODO: maximum text length should be parameter
					button->text = Text::submit(allocator, manager, engine, Text::Specification(64, Font::Font::fromDistanceFieldFile("testGame/res/fonts/consola.sdf")));

					std::vector<Pipeline::Handle> pipelines = ResourceManager::Static::submit(manager,
						std::vector<Pipeline::Specification>({
							Pipeline::Specification(
								std::vector<Shader::Handle>({ button->fragmentShader, button->vertexShader }),
								Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2 })),
								std::vector<DescriptorLayout::Handle>({ button->descriptorLayout }),
								std::vector<Uniform::PushConstant>({ Uniform::PushConstant(Shader::Stage::VERTEX, sizeof(Math::Perspective), 0)})
							)
							}));

					button->pipeline = pipelines[0];

					return button;
				}

				void setup(Button::Handle button, Commands::Context::Handle context, Engine::Handle engine);
				void render(Button::Handle button, Commands::Context::Handle context, Math::Perspective perspective);

				Properties& properties(Button::Handle button);
				void setText(Button::Handle button, Engine::Handle engine, Window::Handle window, Commands::Context::Handle context, const std::string_view view);
			}
		}
	}
}