#pragma once

#include "../../../storage.h"
#include "../../resource_manager.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			namespace Context {
				struct Resource {
					struct {
						Pipeline::Handle pipeline;
						DescriptorLayout::Handle descriptorLayout;
						Shader::Handle fragmentShader;
						Shader::Handle vertexShader;
					} text;

					struct {
						Shader::Handle fragmentShader;
						Shader::Handle vertexShader;

						DescriptorLayout::Handle descriptorLayout;
						Pipeline::Handle pipeline;
					} button;

					// For shaders
					// TODO: possible for descriptor layouts? investigate ocne working
					Allocator::Static::Transient transientStorage;
				};

				typedef Resource* Handle;
				typedef Resource* PromisedHandle;

				template <Allocator::AllocatorType AllocatorType>
				PromisedHandle submit(AllocatorType* allocator, ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window) {
					VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);
					VIVIUM_CHECK_HANDLE_EXISTS(manager);

					Handle handle = Allocator::allocateResource<Resource>(allocator);

					handle->transientStorage = Allocator::Static::Transient(
						sizeof(Shader::Resource) * 4
					);

					// TODO: a bit illegal to steal manager's resource allocator
					handle->text.descriptorLayout = DescriptorLayout::create(&manager->allocationContext.storage, engine, DescriptorLayout::Specification(std::vector<Uniform::Binding>({
						Uniform::Binding(Shader::Stage::FRAGMENT, 0, Uniform::Type::TEXTURE),
						Uniform::Binding(Shader::Stage::FRAGMENT, 1, Uniform::Type::UNIFORM_BUFFER),
						Uniform::Binding(Shader::Stage::VERTEX, 2, Uniform::Type::UNIFORM_BUFFER)
						})));

					handle->button.descriptorLayout = DescriptorLayout::create(&manager->allocationContext.storage, engine, DescriptorLayout::Specification(
						std::vector<Uniform::Binding>({ Uniform::Binding(Shader::Stage::FRAGMENT | Shader::Stage::VERTEX, 0, Uniform::Type::UNIFORM_BUFFER) })
					));

					{
						Shader::Specification fragmentSpecification = Shader::compile(Shader::Stage::FRAGMENT, "testGame/res/text.frag", "testGame/res/text_frag.spv");
						Shader::Specification vertexSpecification = Shader::compile(Shader::Stage::VERTEX, "testGame/res/text.vert", "testGame/res/text_vert.spv");

						handle->text.fragmentShader = Shader::create(&handle->transientStorage, engine, fragmentSpecification);
						handle->text.vertexShader = Shader::create(&handle->transientStorage, engine, vertexSpecification);
					}

					{
						Shader::Specification fragShaderSpec = Shader::compile(Shader::Stage::FRAGMENT, "testGame/res/button.frag", "testGame/res/button_frag.spv");
						Shader::Specification vertShaderSpec = Shader::compile(Shader::Stage::VERTEX, "testGame/res/button.vert", "testGame/res/button_vert.spv");

						handle->button.fragmentShader = Shader::create(&handle->transientStorage, engine, fragShaderSpec);
						handle->button.vertexShader = Shader::create(&handle->transientStorage, engine, vertShaderSpec);
					}

					{
						std::vector<Pipeline::Handle> pipelines = ResourceManager::Static::submit(manager,
							std::vector<Pipeline::Specification>({ Pipeline::Specification::fromWindow(
								std::vector<Shader::Handle>({ handle->text.fragmentShader, handle->text.vertexShader }),
								Buffer::Layout::fromTypes(std::vector<Shader::DataType>({
									Shader::DataType::VEC2,
									Shader::DataType::VEC2
								})),
								std::vector<DescriptorLayout::Handle>({ handle->text.descriptorLayout }),
								std::vector<Uniform::PushConstant>({ Uniform::PushConstant(Shader::Stage::VERTEX, sizeof(Math::Perspective), 0)}),
								engine,
								window
							) }));

						handle->text.pipeline = pipelines[0];
					}

					{
						std::vector<Pipeline::Handle> pipelines = ResourceManager::Static::submit(manager,
							std::vector<Pipeline::Specification>({
								Pipeline::Specification::fromWindow(
									std::vector<Shader::Handle>({ handle->button.fragmentShader, handle->button.vertexShader }),
									Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2 })),
									std::vector<DescriptorLayout::Handle>({ handle->button.descriptorLayout }),
									std::vector<Uniform::PushConstant>({ Uniform::PushConstant(Shader::Stage::VERTEX, sizeof(Math::Perspective), 0)}),
									engine,
									window
								)
								}));

						handle->button.pipeline = pipelines[0];
					}

					return handle;
				}

				void clean(Handle handle, Engine::Handle engine);

				template <Allocator::AllocatorType AllocatorType>
				void drop(AllocatorType* allocator, Handle handle, Engine::Handle engine) {
					VIVIUM_CHECK_HANDLE_EXISTS(handle);

					DescriptorLayout::drop(VIVIUM_RESOURCE_ALLOCATED, handle->text.descriptorLayout, engine);
					Pipeline::drop(VIVIUM_RESOURCE_ALLOCATED, handle->text.pipeline, engine);

					DescriptorLayout::drop(VIVIUM_RESOURCE_ALLOCATED, handle->button.descriptorLayout, engine);
					Pipeline::drop(VIVIUM_RESOURCE_ALLOCATED, handle->button.pipeline, engine);
					
					Allocator::dropResource(allocator, handle);
				}
			}
		}
	}
}