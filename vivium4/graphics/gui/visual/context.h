#pragma once

#include "../../../storage.h"
#include "../../resource_manager.h"
#include "../../color.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			namespace Context {
				struct _ButtonInstanceData {
					F32x2 position;
					F32x2 scale;
					Color foregroundColor;
					float _fill0, _fill1;
				};

				struct Resource {
					struct {
						Pipeline::Handle pipeline;
						DescriptorLayout::Handle descriptorLayout;
						Shader::Handle fragmentShader;
						Shader::Handle vertexShader;
					} text;

					struct {
						static constexpr uint64_t MAX_BUTTONS = 128;

						Shader::Handle fragmentShader;
						Shader::Handle vertexShader;

						Buffer::Handle storageBuffer;

						DescriptorLayout::Handle descriptorLayout;
						DescriptorSet::Handle descriptorSet;
						Pipeline::Handle pipeline;

						Buffer::Handle vertexBuffer;
						Buffer::Handle indexBuffer;
					} button;

					// For shaders
					// TODO: possible for descriptor layouts? investigate ocne working
					Storage::Static::Transient transientStorage;
				};

				typedef Resource* Handle;
				typedef Resource* PromisedHandle;

				template <Storage::StorageType StorageType>
				PromisedHandle submit(StorageType* allocator, ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window) {
					VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);
					VIVIUM_CHECK_HANDLE_EXISTS(manager);

					Handle handle = Storage::allocateResource<Resource>(allocator);

					handle->transientStorage = Storage::Static::Transient(
						sizeof(Shader::Resource) * 6
					);

					ResourceManager::Static::submit(manager, &handle->button.storageBuffer, MemoryType::UNIFORM,
						std::vector<Buffer::Specification>({ Buffer::Specification(handle->button.MAX_BUTTONS * sizeof(_ButtonInstanceData), Buffer::Usage::STORAGE) }));

					std::array<Buffer::Handle, 2> deviceBuffers;

					ResourceManager::Static::submit(manager, deviceBuffers.data(), MemoryType::DEVICE,
						std::vector<Buffer::Specification>({
							Buffer::Specification(4 * sizeof(F32x2), Buffer::Usage::VERTEX),
							Buffer::Specification(6 * sizeof(uint16_t), Buffer::Usage::INDEX)
							}));

					handle->button.vertexBuffer = deviceBuffers[0];
					handle->button.indexBuffer = deviceBuffers[1];

					// TODO: a bit illegal to steal manager's resource allocator
					handle->text.descriptorLayout = DescriptorLayout::create(&manager->resourceAllocator, engine, DescriptorLayout::Specification(std::vector<Uniform::Binding>({
						Uniform::Binding(Shader::Stage::FRAGMENT, 0, Uniform::Type::TEXTURE),
						Uniform::Binding(Shader::Stage::FRAGMENT, 1, Uniform::Type::UNIFORM_BUFFER),
						Uniform::Binding(Shader::Stage::VERTEX, 2, Uniform::Type::UNIFORM_BUFFER)
						})));

					handle->button.descriptorLayout = DescriptorLayout::create(&manager->resourceAllocator, engine, DescriptorLayout::Specification(
						std::vector<Uniform::Binding>({ Uniform::Binding(Shader::Stage::VERTEX, 0, Uniform::Type::STORAGE_BUFFER) })
					));

					ResourceManager::Static::submit(manager, &handle->button.descriptorSet, std::vector<DescriptorSet::Specification>({
						DescriptorSet::Specification(handle->button.descriptorLayout, std::vector<Uniform::Data>({
							Uniform::Data::fromBuffer(handle->button.storageBuffer, handle->button.MAX_BUTTONS * sizeof(_ButtonInstanceData), 0)
							}))
						}));

					{
						Shader::Specification fragmentSpecification = Shader::compileShader(Shader::Stage::FRAGMENT, "vivium4/res/text.frag", "vivium4/res/text_frag.spv");
						Shader::Specification vertexSpecification = Shader::compileShader(Shader::Stage::VERTEX, "vivium4/res/text.vert", "vivium4/res/text_vert.spv");

						handle->text.fragmentShader = Shader::create(&handle->transientStorage, engine, fragmentSpecification);
						handle->text.vertexShader = Shader::create(&handle->transientStorage, engine, vertexSpecification);
					}

					{
						Shader::Specification fragShaderSpec = Shader::compileShader(Shader::Stage::FRAGMENT, "vivium4/res/button.frag", "vivium4/res/button_frag.spv");
						Shader::Specification vertShaderSpec = Shader::compileShader(Shader::Stage::VERTEX, "vivium4/res/button.vert", "vivium4/res/button_vert.spv");

						handle->button.fragmentShader = Shader::create(&handle->transientStorage, engine, fragShaderSpec);
						handle->button.vertexShader = Shader::create(&handle->transientStorage, engine, vertShaderSpec);
					}

					{
						std::array<Pipeline::Handle, 2> pipelines;
						
						ResourceManager::Static::submit(manager,
							pipelines.data(),
							std::vector<Pipeline::Specification>({
							Pipeline::Specification::fromWindow(
								std::vector<Shader::Handle>({ handle->text.fragmentShader, handle->text.vertexShader }),
								Buffer::Layout::fromTypes(std::vector<Shader::DataType>({
									Shader::DataType::VEC2,
									Shader::DataType::VEC2
								})),
								std::vector<DescriptorLayout::Handle>({ handle->text.descriptorLayout }),
								std::vector<Uniform::PushConstant>({ Uniform::PushConstant(Shader::Stage::VERTEX, sizeof(Math::Perspective), 0)}),
								engine,
								window
							),
							Pipeline::Specification::fromWindow(
								std::vector<Shader::Handle>({ handle->button.fragmentShader, handle->button.vertexShader }),
								Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2 })),
								std::vector<DescriptorLayout::Handle>({ handle->button.descriptorLayout }),
								std::vector<Uniform::PushConstant>({ Uniform::PushConstant(Shader::Stage::VERTEX, sizeof(Math::Perspective), 0)}),
								engine,
								window
							)}));

						handle->text.pipeline = pipelines[0];
						handle->button.pipeline = pipelines[1];
					}

					return handle;
				}
				
				void setup(Handle handle, Commands::Context::Handle context, Engine::Handle engine);

				template <Storage::StorageType StorageType>
				void drop(StorageType* allocator, Handle handle, Engine::Handle engine) {
					VIVIUM_CHECK_HANDLE_EXISTS(handle);

					DescriptorLayout::drop(allocator, handle->text.descriptorLayout, engine);
					Pipeline::drop(VIVIUM_NULL_STORAGE, handle->text.pipeline, engine);

					DescriptorLayout::drop(allocator, handle->button.descriptorLayout, engine);
					Pipeline::drop(VIVIUM_NULL_STORAGE, handle->button.pipeline, engine);

					Buffer::drop(VIVIUM_NULL_STORAGE, handle->button.storageBuffer, engine);
					Buffer::drop(VIVIUM_NULL_STORAGE, handle->button.vertexBuffer, engine);
					Buffer::drop(VIVIUM_NULL_STORAGE, handle->button.indexBuffer, engine);
					DescriptorSet::drop(allocator, handle->button.descriptorSet);

					Storage::dropResource(allocator, handle);
				}
			}
		}
	}
}