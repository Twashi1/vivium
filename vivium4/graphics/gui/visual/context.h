#pragma once

#include "../../../storage.h"
#include "../../resource_manager.h"
#include "../../color.h"
#include "../base.h"

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
						Ref<Pipeline> pipeline;
						Ref<DescriptorLayout> descriptorLayout;
						Ref<Shader> fragmentShader;
						Ref<Shader> vertexShader;
					} text;

					struct {
						static constexpr uint64_t MAX_BUTTONS = 128;

						Ref<Shader> fragmentShader;
						Ref<Shader> vertexShader;

						Ref<Buffer> storageBuffer;

						Ref<DescriptorLayout> descriptorLayout;
						Ref<DescriptorSet> descriptorSet;
						Ref<Pipeline> pipeline;

						Ref<Buffer> vertexBuffer;
						Ref<Buffer> indexBuffer;
					} button;

					Storage::Static::Pool elementStorage;
				};

				typedef Resource* Handle;
				typedef Resource* PromisedHandle;

				GUIElement* _allocateGUIElement(Context::Handle context);

				template <Storage::StorageType StorageType>
				PromisedHandle submit(StorageType* allocator, ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window) {
					Handle handle = Storage::allocateResource<Resource>(allocator);

					ResourceManager::Static::submit(manager, &handle->button.storageBuffer.reference, MemoryType::UNIFORM,
						std::vector<BufferSpecification>({ BufferSpecification(handle->button.MAX_BUTTONS * sizeof(_ButtonInstanceData), BufferUsage::STORAGE) }));

					std::array<BufferReference, 2> deviceBuffers;

					ResourceManager::Static::submit(manager, deviceBuffers.data(), MemoryType::DEVICE,
						std::vector<BufferSpecification>({
							BufferSpecification(4 * sizeof(F32x2), BufferUsage::VERTEX),
							BufferSpecification(6 * sizeof(uint16_t), BufferUsage::INDEX)
							}));

					handle->button.vertexBuffer.reference = deviceBuffers[0];
					handle->button.indexBuffer.reference = deviceBuffers[1];

					ResourceManager::Static::submit(manager, &handle->text.descriptorLayout.reference, std::vector<DescriptorLayoutSpecification>({
						DescriptorLayoutSpecification(std::vector<UniformBinding>({
							UniformBinding(ShaderStage::FRAGMENT, 0, UniformType::TEXTURE),
							UniformBinding(ShaderStage::FRAGMENT, 1, UniformType::UNIFORM_BUFFER),
							UniformBinding(ShaderStage::VERTEX, 2, UniformType::UNIFORM_BUFFER)
						}))
					}));

					ResourceManager::Static::submit(manager, &handle->button.descriptorLayout.reference, std::vector<DescriptorLayoutSpecification>({
						DescriptorLayoutSpecification(std::vector<UniformBinding>({
								UniformBinding(ShaderStage::VERTEX, 0, UniformType::STORAGE_BUFFER)
							}))
						}));

					ResourceManager::Static::submit(manager, &handle->button.descriptorSet.reference, std::vector<DescriptorSetSpecification>({
						DescriptorSetSpecification(handle->button.descriptorLayout.reference, std::vector<UniformData>({
							UniformData::fromBuffer(handle->button.storageBuffer.reference, handle->button.MAX_BUTTONS * sizeof(_ButtonInstanceData), 0)
						}))
					}));

					ResourceManager::Static::submit(manager, &handle->text.fragmentShader.reference, std::vector<ShaderSpecification>({
						compileShader(ShaderStage::FRAGMENT, "vivium4/res/text.frag", "vivium4/res/text_frag.spv")
					}));

					ResourceManager::Static::submit(manager, &handle->text.vertexShader.reference, std::vector<ShaderSpecification>({
						compileShader(ShaderStage::VERTEX, "vivium4/res/text.vert", "vivium4/res/text_vert.spv")
					}));

					ResourceManager::Static::submit(manager, &handle->button.fragmentShader.reference, std::vector<ShaderSpecification>({
						compileShader(ShaderStage::FRAGMENT, "vivium4/res/button.frag", "vivium4/res/button_frag.spv")
					}));

					ResourceManager::Static::submit(manager, &handle->button.vertexShader.reference, std::vector<ShaderSpecification>({
						compileShader(ShaderStage::VERTEX, "vivium4/res/button.vert", "vivium4/res/button_vert.spv")
					}));

					{
						std::array<PipelineReference, 2> pipelines;
						
						ResourceManager::Static::submit(manager,
							pipelines.data(),
							std::vector<PipelineSpecification>({
							PipelineSpecification::fromWindow(
								std::vector<ShaderReference>({ handle->text.fragmentShader.reference, handle->text.vertexShader.reference }),
								BufferLayout::fromTypes(std::vector<ShaderDataType>({
									ShaderDataType::VEC2,
									ShaderDataType::VEC2
								})),
								std::vector<DescriptorLayoutReference>({ handle->text.descriptorLayout.reference }),
								std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Math::Perspective) )}),
								engine,
								window
							),
							PipelineSpecification::fromWindow(
								std::vector<ShaderReference>({ handle->button.fragmentShader.reference, handle->button.vertexShader.reference }),
								BufferLayout::fromTypes(std::vector<ShaderDataType>({ ShaderDataType::VEC2 })),
								std::vector<DescriptorLayoutReference>({ handle->button.descriptorLayout.reference }),
								std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Math::Perspective) )}),
								engine,
								window
							)}));

						handle->text.pipeline.reference = pipelines[0];
						handle->button.pipeline.reference = pipelines[1];
					}

					return handle;
				}
				
				void setup(Handle handle, ResourceManager::Static::Handle manager, Commands::Context::Handle context, Engine::Handle engine);

				template <Storage::StorageType StorageType>
				void drop(StorageType* allocator, Handle handle, Engine::Handle engine) {
					VIVIUM_CHECK_HANDLE_EXISTS(handle);

					dropDescriptorLayout(handle->text.descriptorLayout.resource, engine);
					dropPipeline(handle->text.pipeline.resource, engine);

					dropDescriptorLayout(handle->button.descriptorLayout.resource, engine);
					dropPipeline(handle->button.pipeline.resource, engine);

					dropBuffer(handle->button.storageBuffer.resource, engine);
					dropBuffer(handle->button.vertexBuffer.resource, engine);
					dropBuffer(handle->button.indexBuffer.resource, engine);

					Storage::dropResource(allocator, handle);
				}
			}
		}
	}
}