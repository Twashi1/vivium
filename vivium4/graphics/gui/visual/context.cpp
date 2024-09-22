#include "context.h"

namespace Vivium {
	void _submitGenericGUIContext(GUIContext& guiContext, ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window)
	{
		// Create null element
		guiContext.defaultParent = createGUIElement(guiContext);

		std::array<BufferReference, 2> deviceBuffers;

		ResourceManager::Static::submit(manager, deviceBuffers.data(), MemoryType::DEVICE,
			std::vector<BufferSpecification>({
				BufferSpecification(4 * sizeof(F32x2), BufferUsage::VERTEX),
				BufferSpecification(6 * sizeof(uint16_t), BufferUsage::INDEX)
				}));

		guiContext.rectVertexBuffer.reference = deviceBuffers[0];
		guiContext.rectIndexBuffer.reference = deviceBuffers[1];
	}

	void _submitTextGUIContext(GUIContext& guiContext, ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window)
	{
		guiContext.text.bufferLayout = BufferLayout::fromTypes(std::vector<ShaderDataType>({
			ShaderDataType::VEC2,
			ShaderDataType::VEC2,
			ShaderDataType::VEC3
		}));

		ResourceManager::Static::submit(manager, &guiContext.text.descriptorLayout.reference, std::vector<DescriptorLayoutSpecification>({
			DescriptorLayoutSpecification(std::vector<UniformBinding>({
				UniformBinding(ShaderStage::FRAGMENT, 0, UniformType::TEXTURE)
			}))
			}));

		ResourceManager::Static::submit(manager, &guiContext.text.fragmentShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::FRAGMENT, "vivium4/res/text.frag", "vivium4/res/text_frag.spv")
			}));

		ResourceManager::Static::submit(manager, &guiContext.text.vertexShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::VERTEX, "vivium4/res/text.vert", "vivium4/res/text_vert.spv")
			}));

		ResourceManager::Static::submit(manager,
			&guiContext.text.pipeline.reference,
			std::vector<PipelineSpecification>({
			PipelineSpecification::fromWindow(
				std::vector<ShaderReference>({ guiContext.text.fragmentShader.reference, guiContext.text.vertexShader.reference }),
				guiContext.text.bufferLayout,
				std::vector<DescriptorLayoutReference>({ guiContext.text.descriptorLayout.reference }),
				std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Math::Perspective))}),
				engine,
				window
			)
		}));
	}

	void _submitButtonGUIContext(GUIContext& guiContext, ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window)
	{
		ResourceManager::Static::submit(manager, &guiContext.button.storageBuffer.reference, MemoryType::UNIFORM,
			std::vector<BufferSpecification>({ BufferSpecification(guiContext.button.MAX_BUTTONS * sizeof(_GUIButtonInstanceData), BufferUsage::STORAGE) }));

		ResourceManager::Static::submit(manager, &guiContext.button.descriptorLayout.reference, std::vector<DescriptorLayoutSpecification>({
			DescriptorLayoutSpecification(std::vector<UniformBinding>({
					UniformBinding(ShaderStage::VERTEX, 0, UniformType::STORAGE_BUFFER)
				}))
			}));

		ResourceManager::Static::submit(manager, &guiContext.button.descriptorSet.reference, std::vector<DescriptorSetSpecification>({
			DescriptorSetSpecification(guiContext.button.descriptorLayout.reference, std::vector<UniformData>({
				UniformData::fromBuffer(guiContext.button.storageBuffer.reference, guiContext.button.MAX_BUTTONS * sizeof(_GUIButtonInstanceData), 0)
			}))
			}));

		ResourceManager::Static::submit(manager, &guiContext.button.fragmentShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::FRAGMENT, "vivium4/res/button.frag", "vivium4/res/button_frag.spv")
			}));

		ResourceManager::Static::submit(manager, &guiContext.button.vertexShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::VERTEX, "vivium4/res/button.vert", "vivium4/res/button_vert.spv")
			}));

		ResourceManager::Static::submit(manager,
			&guiContext.button.pipeline.reference,
			std::vector<PipelineSpecification>({
			PipelineSpecification::fromWindow(
				std::vector<ShaderReference>({ guiContext.button.fragmentShader.reference, guiContext.button.vertexShader.reference }),
				BufferLayout::fromTypes(std::vector<ShaderDataType>({ ShaderDataType::VEC2 })),
				std::vector<DescriptorLayoutReference>({ guiContext.button.descriptorLayout.reference }),
				std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Math::Perspective))}),
				engine,
				window
			) }));
	}

	void _submitPanelGUIContext(GUIContext& guiContext, ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window)
	{
		ResourceManager::Static::submit(manager, &guiContext.panel.storageBuffer.reference, MemoryType::UNIFORM,
			std::vector<BufferSpecification>({ BufferSpecification(guiContext.panel.MAX_PANELS * sizeof(_GUIPanelInstanceData), BufferUsage::STORAGE) }));

		ResourceManager::Static::submit(manager, &guiContext.panel.descriptorLayout.reference, std::vector<DescriptorLayoutSpecification>({
			DescriptorLayoutSpecification(std::vector<UniformBinding>({
					UniformBinding(ShaderStage::VERTEX, 0, UniformType::STORAGE_BUFFER)
				}))
			}));

		ResourceManager::Static::submit(manager, &guiContext.panel.descriptorSet.reference, std::vector<DescriptorSetSpecification>({
			DescriptorSetSpecification(guiContext.panel.descriptorLayout.reference, std::vector<UniformData>({
				UniformData::fromBuffer(guiContext.panel.storageBuffer.reference, guiContext.panel.MAX_PANELS * sizeof(_GUIPanelInstanceData), 0)
			}))
			}));

		ResourceManager::Static::submit(manager, &guiContext.panel.fragmentShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::FRAGMENT, "vivium4/res/panel.frag", "vivium4/res/panel_frag.spv")
			}));

		ResourceManager::Static::submit(manager, &guiContext.panel.vertexShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::VERTEX, "vivium4/res/panel.vert", "vivium4/res/panel_vert.spv")
			}));

		ResourceManager::Static::submit(manager,
			&guiContext.panel.pipeline.reference,
			std::vector<PipelineSpecification>({
			PipelineSpecification::fromWindow(
				std::vector<ShaderReference>({ guiContext.panel.fragmentShader.reference, guiContext.panel.vertexShader.reference }),
				BufferLayout::fromTypes(std::vector<ShaderDataType>({ ShaderDataType::VEC2 })),
				std::vector<DescriptorLayoutReference>({ guiContext.panel.descriptorLayout.reference }),
				std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Math::Perspective))}),
				engine,
				window
			) }));
	}

	GUIElementReference createGUIElement(GUIContext& guiContext)
	{
		guiContext.guiElements.push_back(GUIElement());

		return GUIElementReference(guiContext.guiElements.size() - 1);
	}

	GUIElementReference defaultGUIParent(GUIContext& context)
	{
		return context.defaultParent;
	}

	// TODO: create doesn't match the pattern, elements that require a setup, should also be `submit`
	GUIContext createGUIContext(ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window) {
		// TODO: move the code, should be done in some initialisation function
		// Generate the font if it doesn't exist
		if (!std::filesystem::exists("vivium4/res/fonts/consola.sdf"))
		{
			Font::compileSignedDistanceField("vivium4/res/fonts/consola.ttf", 512, "vivium4/res/fonts/consola.sdf", 48, 1.0f);
		}

		GUIContext context;

		_submitGenericGUIContext(context, manager, engine, window);
		_submitTextGUIContext(context, manager, engine, window);
		_submitButtonGUIContext(context, manager, engine, window);
		_submitPanelGUIContext(context, manager, engine, window);

		return context;
	}

	void setupGUIContext(GUIContext& guiContext, ResourceManager::Static::Handle manager, Commands::Context::Handle context, Engine::Handle engine)
	{
		ResourceManager::Static::convertReference(manager, guiContext.text.pipeline);
		ResourceManager::Static::convertReference(manager, guiContext.text.descriptorLayout);
		ResourceManager::Static::convertReference(manager, guiContext.text.fragmentShader);
		ResourceManager::Static::convertReference(manager, guiContext.text.vertexShader);

		ResourceManager::Static::convertReference(manager, guiContext.button.pipeline);
		ResourceManager::Static::convertReference(manager, guiContext.button.descriptorLayout);
		ResourceManager::Static::convertReference(manager, guiContext.button.fragmentShader);
		ResourceManager::Static::convertReference(manager, guiContext.button.vertexShader);
		ResourceManager::Static::convertReference(manager, guiContext.button.storageBuffer);
		ResourceManager::Static::convertReference(manager, guiContext.button.descriptorSet);

		ResourceManager::Static::convertReference(manager, guiContext.panel.pipeline);
		ResourceManager::Static::convertReference(manager, guiContext.panel.descriptorLayout);
		ResourceManager::Static::convertReference(manager, guiContext.panel.fragmentShader);
		ResourceManager::Static::convertReference(manager, guiContext.panel.vertexShader);
		ResourceManager::Static::convertReference(manager, guiContext.panel.storageBuffer);
		ResourceManager::Static::convertReference(manager, guiContext.panel.descriptorSet);

		ResourceManager::Static::convertReference(manager, guiContext.rectVertexBuffer);
		ResourceManager::Static::convertReference(manager, guiContext.rectIndexBuffer);

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
		Commands::transferBuffer(context, resource, 8 * sizeof(float), 0, guiContext.rectVertexBuffer.resource);

		std::memcpy(reinterpret_cast<uint8_t*>(stagingMapping) + 8 * sizeof(float), indexData, 6 * sizeof(uint16_t));
		Commands::transferBuffer(context, resource, 6 * sizeof(uint16_t), 8 * sizeof(float), guiContext.rectIndexBuffer.resource);

		Commands::Context::endTransfer(context, engine);

		Commands::freeOneTimeStagingBuffer(engine, stagingBuffer, temporaryMemory);

		dropShader(guiContext.text.fragmentShader.resource, engine);
		dropShader(guiContext.text.vertexShader.resource, engine);

		dropShader(guiContext.button.fragmentShader.resource, engine);
		dropShader(guiContext.button.vertexShader.resource, engine);

		dropShader(guiContext.panel.fragmentShader.resource, engine);
		dropShader(guiContext.panel.vertexShader.resource, engine);

		// TODO: maybe the descriptor layout can be freed here?
	}

	void updateGUIContext(GUIContext& guiContext, F32x2 windowDimensions)
	{
		// TODO: update all children of the default parent
		updateGUIElement(GUIElementReference(NULL), GUIElementReference(NULL), windowDimensions, guiContext);
	}

	void dropGUIContext(GUIContext& guiContext, Engine::Handle engine) {
		guiContext.guiElements = {};

		dropDescriptorLayout(guiContext.text.descriptorLayout.resource, engine);
		dropPipeline(guiContext.text.pipeline.resource, engine);

		dropDescriptorLayout(guiContext.button.descriptorLayout.resource, engine);
		dropDescriptorLayout(guiContext.panel.descriptorLayout.resource, engine);
		dropPipeline(guiContext.button.pipeline.resource, engine);
		dropPipeline(guiContext.panel.pipeline.resource, engine);

		dropBuffer(guiContext.button.storageBuffer.resource, engine);
		dropBuffer(guiContext.panel.storageBuffer.resource, engine);

		dropBuffer(guiContext.rectVertexBuffer.resource, engine);
		dropBuffer(guiContext.rectIndexBuffer.resource, engine);
	}
}