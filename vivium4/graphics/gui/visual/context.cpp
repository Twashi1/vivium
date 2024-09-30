#include "context.h"

namespace Vivium {
	void _submitGenericGUIContext(GUIContext& guiContext, ResourceManager& manager, Engine& engine, Window& window)
	{
		// Create null element
		guiContext.defaultParent = createGUIElement(guiContext);

		std::array<BufferReference, 2> deviceBuffers;

		submitResource(manager, deviceBuffers.data(), MemoryType::DEVICE,
			std::vector<BufferSpecification>({
				BufferSpecification(4 * sizeof(F32x2), BufferUsage::VERTEX),
				BufferSpecification(6 * sizeof(uint16_t), BufferUsage::INDEX)
				}));

		guiContext.rectVertexBuffer.reference = deviceBuffers[0];
		guiContext.rectIndexBuffer.reference = deviceBuffers[1];
	}

	void _submitTextGUIContext(GUIContext& guiContext, ResourceManager& manager, Engine& engine, Window& window)
	{
		guiContext.text.bufferLayout = BufferLayout::fromTypes(std::vector<ShaderDataType>({
			ShaderDataType::VEC2,
			ShaderDataType::VEC2,
			ShaderDataType::VEC3
		}));

		submitResource(manager, &guiContext.text.descriptorLayout.reference, std::vector<DescriptorLayoutSpecification>({
			DescriptorLayoutSpecification(std::vector<UniformBinding>({
				UniformBinding(ShaderStage::FRAGMENT, 0, UniformType::TEXTURE)
			}))
			}));

		submitResource(manager, &guiContext.text.fragmentShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::FRAGMENT, "vivium4/res/text.frag", "vivium4/res/text_frag.spv")
			}));

		submitResource(manager, &guiContext.text.vertexShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::VERTEX, "vivium4/res/text.vert", "vivium4/res/text_vert.spv")
			}));

		submitResource(manager,
			&guiContext.text.pipeline.reference,
			std::vector<PipelineSpecification>({
			PipelineSpecification::fromWindow(
				std::vector<ShaderReference>({ guiContext.text.fragmentShader.reference, guiContext.text.vertexShader.reference }),
				guiContext.text.bufferLayout,
				std::vector<DescriptorLayoutReference>({ guiContext.text.descriptorLayout.reference }),
				std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Math::Perspective))}),
				window
			)
		}));
	}

	void _submitButtonGUIContext(GUIContext& guiContext, ResourceManager& manager, Engine& engine, Window& window)
	{
		submitResource(manager, &guiContext.button.storageBuffer.reference, MemoryType::UNIFORM,
			std::vector<BufferSpecification>({ BufferSpecification(guiContext.button.MAX_BUTTONS * sizeof(_GUIButtonInstanceData), BufferUsage::STORAGE) }));

		submitResource(manager, &guiContext.button.descriptorLayout.reference, std::vector<DescriptorLayoutSpecification>({
			DescriptorLayoutSpecification(std::vector<UniformBinding>({
					UniformBinding(ShaderStage::VERTEX, 0, UniformType::STORAGE_BUFFER)
				}))
			}));

		submitResource(manager, &guiContext.button.descriptorSet.reference, std::vector<DescriptorSetSpecification>({
			DescriptorSetSpecification(guiContext.button.descriptorLayout.reference, std::vector<UniformData>({
				UniformData::fromBuffer(guiContext.button.storageBuffer.reference, guiContext.button.MAX_BUTTONS * sizeof(_GUIButtonInstanceData), 0)
			}))
			}));

		submitResource(manager, &guiContext.button.fragmentShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::FRAGMENT, "vivium4/res/button.frag", "vivium4/res/button_frag.spv")
			}));

		submitResource(manager, &guiContext.button.vertexShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::VERTEX, "vivium4/res/button.vert", "vivium4/res/button_vert.spv")
			}));

		submitResource(manager,
			&guiContext.button.pipeline.reference,
			std::vector<PipelineSpecification>({
			PipelineSpecification::fromWindow(
				std::vector<ShaderReference>({ guiContext.button.fragmentShader.reference, guiContext.button.vertexShader.reference }),
				BufferLayout::fromTypes(std::vector<ShaderDataType>({ ShaderDataType::VEC2 })),
				std::vector<DescriptorLayoutReference>({ guiContext.button.descriptorLayout.reference }),
				std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Math::Perspective))}),
				window
			) }));
	}

	void _submitPanelGUIContext(GUIContext& guiContext, ResourceManager& manager, Engine& engine, Window& window)
	{
		submitResource(manager, &guiContext.panel.storageBuffer.reference, MemoryType::UNIFORM,
			std::vector<BufferSpecification>({ BufferSpecification(guiContext.panel.MAX_PANELS * sizeof(_GUIPanelInstanceData), BufferUsage::STORAGE) }));

		submitResource(manager, &guiContext.panel.descriptorLayout.reference, std::vector<DescriptorLayoutSpecification>({
			DescriptorLayoutSpecification(std::vector<UniformBinding>({
					UniformBinding(ShaderStage::VERTEX, 0, UniformType::STORAGE_BUFFER)
				}))
			}));

		submitResource(manager, &guiContext.panel.descriptorSet.reference, std::vector<DescriptorSetSpecification>({
			DescriptorSetSpecification(guiContext.panel.descriptorLayout.reference, std::vector<UniformData>({
				UniformData::fromBuffer(guiContext.panel.storageBuffer.reference, guiContext.panel.MAX_PANELS * sizeof(_GUIPanelInstanceData), 0)
			}))
			}));

		submitResource(manager, &guiContext.panel.fragmentShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::FRAGMENT, "vivium4/res/panel.frag", "vivium4/res/panel_frag.spv")
			}));

		submitResource(manager, &guiContext.panel.vertexShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::VERTEX, "vivium4/res/panel.vert", "vivium4/res/panel_vert.spv")
			}));

		submitResource(manager,
			&guiContext.panel.pipeline.reference,
			std::vector<PipelineSpecification>({
			PipelineSpecification::fromWindow(
				std::vector<ShaderReference>({ guiContext.panel.fragmentShader.reference, guiContext.panel.vertexShader.reference }),
				BufferLayout::fromTypes(std::vector<ShaderDataType>({ ShaderDataType::VEC2 })),
				std::vector<DescriptorLayoutReference>({ guiContext.panel.descriptorLayout.reference }),
				std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Math::Perspective))}),
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
	GUIContext createGUIContext(ResourceManager& manager, Engine& engine, Window& window) {
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

	void setupGUIContext(GUIContext& guiContext, ResourceManager& manager, CommandContext& context, Engine& engine)
	{
		convertResourceReference(manager, guiContext.text.pipeline);
		convertResourceReference(manager, guiContext.text.descriptorLayout);
		convertResourceReference(manager, guiContext.text.fragmentShader);
		convertResourceReference(manager, guiContext.text.vertexShader);

		convertResourceReference(manager, guiContext.button.pipeline);
		convertResourceReference(manager, guiContext.button.descriptorLayout);
		convertResourceReference(manager, guiContext.button.fragmentShader);
		convertResourceReference(manager, guiContext.button.vertexShader);
		convertResourceReference(manager, guiContext.button.storageBuffer);
		convertResourceReference(manager, guiContext.button.descriptorSet);

		convertResourceReference(manager, guiContext.panel.pipeline);
		convertResourceReference(manager, guiContext.panel.descriptorLayout);
		convertResourceReference(manager, guiContext.panel.fragmentShader);
		convertResourceReference(manager, guiContext.panel.vertexShader);
		convertResourceReference(manager, guiContext.panel.storageBuffer);
		convertResourceReference(manager, guiContext.panel.descriptorSet);

		convertResourceReference(manager, guiContext.rectVertexBuffer);
		convertResourceReference(manager, guiContext.rectIndexBuffer);

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
		_cmdCreateTransientStagingBuffer(engine, &stagingBuffer, &temporaryMemory,
			8 * sizeof(float) + 6 * sizeof(uint16_t), &stagingMapping);

		Buffer resource;
		resource.buffer = stagingBuffer;
		resource.mapping = stagingMapping;

		contextBeginTransfer(context);

		std::memcpy(stagingMapping, vertexData, 8 * sizeof(float));
		cmdTransferBuffer(context, resource, 8 * sizeof(float), 0, guiContext.rectVertexBuffer.resource);

		std::memcpy(reinterpret_cast<uint8_t*>(stagingMapping) + 8 * sizeof(float), indexData, 6 * sizeof(uint16_t));
		cmdTransferBuffer(context, resource, 6 * sizeof(uint16_t), 8 * sizeof(float), guiContext.rectIndexBuffer.resource);

		contextEndTransfer(context, engine);

		_cmdFreeTransientStagingBuffer(engine, stagingBuffer, temporaryMemory);

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

	void dropGUIContext(GUIContext& guiContext, Engine& engine) {
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