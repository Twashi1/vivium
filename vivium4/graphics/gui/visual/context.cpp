#include "context.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			namespace Context {
				void _submitGenericContext(Handle handle, ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window)
				{
					std::array<BufferReference, 2> deviceBuffers;

					ResourceManager::Static::submit(manager, deviceBuffers.data(), MemoryType::DEVICE,
						std::vector<BufferSpecification>({
							BufferSpecification(4 * sizeof(F32x2), BufferUsage::VERTEX),
							BufferSpecification(6 * sizeof(uint16_t), BufferUsage::INDEX)
							}));

					handle->rectVertexBuffer.reference = deviceBuffers[0];
					handle->rectIndexBuffer.reference = deviceBuffers[1];
				}

				void _submitTextContext(Handle handle, ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window)
				{
					ResourceManager::Static::submit(manager, &handle->text.descriptorLayout.reference, std::vector<DescriptorLayoutSpecification>({
						DescriptorLayoutSpecification(std::vector<UniformBinding>({
							UniformBinding(ShaderStage::FRAGMENT, 0, UniformType::TEXTURE),
							UniformBinding(ShaderStage::FRAGMENT, 1, UniformType::UNIFORM_BUFFER),
							UniformBinding(ShaderStage::VERTEX, 2, UniformType::UNIFORM_BUFFER)
						}))
						}));

					ResourceManager::Static::submit(manager, &handle->text.fragmentShader.reference, std::vector<ShaderSpecification>({
						compileShader(ShaderStage::FRAGMENT, "vivium4/res/text.frag", "vivium4/res/text_frag.spv")
						}));

					ResourceManager::Static::submit(manager, &handle->text.vertexShader.reference, std::vector<ShaderSpecification>({
						compileShader(ShaderStage::VERTEX, "vivium4/res/text.vert", "vivium4/res/text_vert.spv")
						}));

					ResourceManager::Static::submit(manager,
						&handle->text.pipeline.reference,
						std::vector<PipelineSpecification>({
						PipelineSpecification::fromWindow(
							std::vector<ShaderReference>({ handle->text.fragmentShader.reference, handle->text.vertexShader.reference }),
							BufferLayout::fromTypes(std::vector<ShaderDataType>({
								ShaderDataType::VEC2,
								ShaderDataType::VEC2
							})),
							std::vector<DescriptorLayoutReference>({ handle->text.descriptorLayout.reference }),
							std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Math::Perspective))}),
							engine,
							window
						)
							}));
				}

				void _submitButtonContext(Handle handle, ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window)
				{
					ResourceManager::Static::submit(manager, &handle->button.storageBuffer.reference, MemoryType::UNIFORM,
						std::vector<BufferSpecification>({ BufferSpecification(handle->button.MAX_BUTTONS * sizeof(_ButtonInstanceData), BufferUsage::STORAGE) }));

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

					ResourceManager::Static::submit(manager, &handle->button.fragmentShader.reference, std::vector<ShaderSpecification>({
						compileShader(ShaderStage::FRAGMENT, "vivium4/res/button.frag", "vivium4/res/button_frag.spv")
						}));

					ResourceManager::Static::submit(manager, &handle->button.vertexShader.reference, std::vector<ShaderSpecification>({
						compileShader(ShaderStage::VERTEX, "vivium4/res/button.vert", "vivium4/res/button_vert.spv")
						}));

					ResourceManager::Static::submit(manager,
						&handle->button.pipeline.reference,
						std::vector<PipelineSpecification>({
						PipelineSpecification::fromWindow(
							std::vector<ShaderReference>({ handle->button.fragmentShader.reference, handle->button.vertexShader.reference }),
							BufferLayout::fromTypes(std::vector<ShaderDataType>({ ShaderDataType::VEC2 })),
							std::vector<DescriptorLayoutReference>({ handle->button.descriptorLayout.reference }),
							std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Math::Perspective))}),
							engine,
							window
						) }));
				}

				void _submitPanelContext(Handle handle, ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window)
				{
					ResourceManager::Static::submit(manager, &handle->panel.storageBuffer.reference, MemoryType::UNIFORM,
						std::vector<BufferSpecification>({ BufferSpecification(handle->panel.MAX_PANELS * sizeof(_PanelInstanceData), BufferUsage::STORAGE) }));

					ResourceManager::Static::submit(manager, &handle->panel.descriptorLayout.reference, std::vector<DescriptorLayoutSpecification>({
						DescriptorLayoutSpecification(std::vector<UniformBinding>({
								UniformBinding(ShaderStage::VERTEX, 0, UniformType::STORAGE_BUFFER)
							}))
						}));

					ResourceManager::Static::submit(manager, &handle->panel.descriptorSet.reference, std::vector<DescriptorSetSpecification>({
						DescriptorSetSpecification(handle->panel.descriptorLayout.reference, std::vector<UniformData>({
							UniformData::fromBuffer(handle->panel.storageBuffer.reference, handle->panel.MAX_PANELS * sizeof(_PanelInstanceData), 0)
						}))
						}));

					ResourceManager::Static::submit(manager, &handle->panel.fragmentShader.reference, std::vector<ShaderSpecification>({
						compileShader(ShaderStage::FRAGMENT, "vivium4/res/panel.frag", "vivium4/res/panel_frag.spv")
						}));

					ResourceManager::Static::submit(manager, &handle->panel.vertexShader.reference, std::vector<ShaderSpecification>({
						compileShader(ShaderStage::VERTEX, "vivium4/res/panel.vert", "vivium4/res/panel_vert.spv")
						}));

					ResourceManager::Static::submit(manager,
						&handle->panel.pipeline.reference,
						std::vector<PipelineSpecification>({
						PipelineSpecification::fromWindow(
							std::vector<ShaderReference>({ handle->panel.fragmentShader.reference, handle->panel.vertexShader.reference }),
							BufferLayout::fromTypes(std::vector<ShaderDataType>({ ShaderDataType::VEC2 })),
							std::vector<DescriptorLayoutReference>({ handle->panel.descriptorLayout.reference }),
							std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Math::Perspective))}),
							engine,
							window
						) }));
				}

				GUIElement* _allocateGUIElement(Context::Handle context)
				{
					return Storage::allocateResource<GUIElement>(&context->elementStorage);
				}

				void setup(Handle handle, ResourceManager::Static::Handle manager, Commands::Context::Handle context, Engine::Handle engine)
				{
					ResourceManager::Static::convertReference(manager, handle->text.pipeline);
					ResourceManager::Static::convertReference(manager, handle->text.descriptorLayout);
					ResourceManager::Static::convertReference(manager, handle->text.fragmentShader);
					ResourceManager::Static::convertReference(manager, handle->text.vertexShader);

					ResourceManager::Static::convertReference(manager, handle->button.pipeline);
					ResourceManager::Static::convertReference(manager, handle->button.descriptorLayout);
					ResourceManager::Static::convertReference(manager, handle->button.fragmentShader);
					ResourceManager::Static::convertReference(manager, handle->button.vertexShader);
					ResourceManager::Static::convertReference(manager, handle->button.storageBuffer);
					ResourceManager::Static::convertReference(manager, handle->button.descriptorSet);

					ResourceManager::Static::convertReference(manager, handle->panel.pipeline);
					ResourceManager::Static::convertReference(manager, handle->panel.descriptorLayout);
					ResourceManager::Static::convertReference(manager, handle->panel.fragmentShader);
					ResourceManager::Static::convertReference(manager, handle->panel.vertexShader);
					ResourceManager::Static::convertReference(manager, handle->panel.storageBuffer);
					ResourceManager::Static::convertReference(manager, handle->panel.descriptorSet);

					ResourceManager::Static::convertReference(manager, handle->rectVertexBuffer);
					ResourceManager::Static::convertReference(manager, handle->rectIndexBuffer);

					float vertexData[] = {
						0.0f, 0.0f,
						1.0f, 0.0f,
						1.0f, 1.0f,
						0.0f, 1.0f
					};

					uint16_t indexData[] = { 0, 1, 2, 2, 3, 0 };

					// TODO: big error here, we schedule two moves from the same staging buffer, resulting in
					// an overwrite that gives bogus data!

					VkDeviceMemory temporaryMemory;
					VkBuffer stagingBuffer;
					void* stagingMapping;
					Commands::createOneTimeStagingBuffer(engine, &stagingBuffer, &temporaryMemory,
						8 * sizeof(float) + 6 * sizeof(uint16_t), &stagingMapping);

					Buffer resource;
					resource.buffer = stagingBuffer;
					resource.mapping = stagingMapping;

					Commands::Context::beginTransfer(context);

					std::memcpy(stagingMapping, vertexData, 8 * sizeof(float));
					Commands::transferBuffer(context, resource, 8 * sizeof(float), 0, handle->rectVertexBuffer.resource);

					std::memcpy(reinterpret_cast<uint8_t*>(stagingMapping) + 8 * sizeof(float), indexData, 6 * sizeof(uint16_t));
					Commands::transferBuffer(context, resource, 6 * sizeof(uint16_t), 8 * sizeof(float), handle->rectIndexBuffer.resource);

					Commands::Context::endTransfer(context, engine);

					Commands::freeOneTimeStagingBuffer(engine, stagingBuffer, temporaryMemory);

					dropShader(handle->text.fragmentShader.resource, engine);
					dropShader(handle->text.vertexShader.resource, engine);

					dropShader(handle->button.fragmentShader.resource, engine);
					dropShader(handle->button.vertexShader.resource, engine);

					dropShader(handle->panel.fragmentShader.resource, engine);
					dropShader(handle->panel.vertexShader.resource, engine);

					// TODO: maybe the descriptor layout can be freed here?
				}
			}
		}
	}
}