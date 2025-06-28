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
				std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Perspective))}),
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
				std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Perspective))}),
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
				std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Perspective))}),
				window
			) }));
	}

	void _submitSliderGUIContext(GUIContext& guiContext, ResourceManager& manager, Engine& engine, Window& window)
	{
		submitResource(manager, &guiContext.slider.storageBuffer.reference, MemoryType::UNIFORM,
			std::vector<BufferSpecification>({ BufferSpecification(guiContext.slider.MAX_SLIDERS * sizeof(_GUISliderInstanceData), BufferUsage::STORAGE) }));

		submitResource(manager, &guiContext.slider.descriptorLayout.reference, std::vector<DescriptorLayoutSpecification>({
			DescriptorLayoutSpecification(std::vector<UniformBinding>({
					UniformBinding(ShaderStage::VERTEX, 0, UniformType::STORAGE_BUFFER)
				}))
			}));

		submitResource(manager, &guiContext.slider.descriptorSet.reference, std::vector<DescriptorSetSpecification>({
			DescriptorSetSpecification(guiContext.slider.descriptorLayout.reference, std::vector<UniformData>({
				UniformData::fromBuffer(guiContext.slider.storageBuffer.reference, guiContext.slider.MAX_SLIDERS * sizeof(_GUISliderInstanceData), 0)
			}))
			}));

		submitResource(manager, &guiContext.slider.fragmentShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::FRAGMENT, "vivium4/res/slider.frag", "vivium4/res/slider_frag.spv")
			}));

		submitResource(manager, &guiContext.slider.vertexShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::VERTEX, "vivium4/res/slider.vert", "vivium4/res/slider_vert.spv")
			}));

		submitResource(manager,
			&guiContext.slider.pipeline.reference,
			std::vector<PipelineSpecification>({
			PipelineSpecification::fromWindow(
				std::vector<ShaderReference>({ guiContext.slider.fragmentShader.reference, guiContext.slider.vertexShader.reference }),
				BufferLayout::fromTypes(std::vector<ShaderDataType>({ ShaderDataType::VEC2 })),
				std::vector<DescriptorLayoutReference>({ guiContext.slider.descriptorLayout.reference }),
				std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Perspective))}),
				window
			) }));
	}

	void _submitSpriteGUIContext(GUIContext& guiContext, ResourceManager& manager, Engine& engine, Window& window)
	{
		submitResource(manager, &guiContext.sprite.storageBuffer.reference, MemoryType::UNIFORM,
			std::vector<BufferSpecification>({ BufferSpecification(guiContext.sprite.MAX_SPRITES * sizeof(_GUISpriteInstanceData), BufferUsage::STORAGE) }));

		submitResource(manager, &guiContext.sprite.descriptorLayout.reference, std::vector<DescriptorLayoutSpecification>({
			DescriptorLayoutSpecification(std::vector<UniformBinding>({
					UniformBinding(ShaderStage::VERTEX, 0, UniformType::STORAGE_BUFFER),
					UniformBinding(ShaderStage::FRAGMENT, 1, UniformType::TEXTURE),
				}))
			}));

		submitResource(manager, &guiContext.sprite.texture.reference, std::vector<TextureSpecification>({
			TextureSpecification::fromData(guiContext.sprite.atlas->data, guiContext.sprite.atlas->size, guiContext.sprite.atlas->format, TextureFilter::NEAREST)
			}));

		submitResource(manager, &guiContext.sprite.descriptorSet.reference, std::vector<DescriptorSetSpecification>({
			DescriptorSetSpecification(guiContext.sprite.descriptorLayout.reference, std::vector<UniformData>({
				UniformData::fromBuffer(guiContext.sprite.storageBuffer.reference, guiContext.sprite.MAX_SPRITES * sizeof(_GUISpriteInstanceData), 0),
				UniformData::fromTexture(guiContext.sprite.texture.reference)
			}))
			}));

		submitResource(manager, &guiContext.sprite.fragmentShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::FRAGMENT, "vivium4/res/sprite.frag", "vivium4/res/sprite_frag.spv")
			}));

		submitResource(manager, &guiContext.sprite.vertexShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::VERTEX, "vivium4/res/sprite.vert", "vivium4/res/sprite_vert.spv")
			}));

		submitResource(manager,
			&guiContext.sprite.pipeline.reference,
			std::vector<PipelineSpecification>({
			PipelineSpecification::fromWindow(
				std::vector<ShaderReference>({ guiContext.sprite.fragmentShader.reference, guiContext.sprite.vertexShader.reference }),
				BufferLayout::fromTypes(std::vector<ShaderDataType>({ ShaderDataType::VEC2 })),
				std::vector<DescriptorLayoutReference>({ guiContext.sprite.descriptorLayout.reference }),
				std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Perspective))}),
				window
			) }));
	}

	void _submitDebugRectGUIContext(GUIContext& guiContext, ResourceManager& manager, Engine& engine, Window& window)
	{
		submitResource(manager, &guiContext.debugRect.storageBuffer.reference, MemoryType::UNIFORM,
			std::vector<BufferSpecification>({ BufferSpecification(guiContext.debugRect.MAX_DEBUG_RECTS * sizeof(_GUIDebugRectInstanceData), BufferUsage::STORAGE) }));

		submitResource(manager, &guiContext.debugRect.descriptorLayout.reference, std::vector<DescriptorLayoutSpecification>({
			DescriptorLayoutSpecification(std::vector<UniformBinding>({
					UniformBinding(ShaderStage::VERTEX, 0, UniformType::STORAGE_BUFFER)
				}))
			}));

		submitResource(manager, &guiContext.debugRect.descriptorSet.reference, std::vector<DescriptorSetSpecification>({
			DescriptorSetSpecification(guiContext.debugRect.descriptorLayout.reference, std::vector<UniformData>({
				UniformData::fromBuffer(guiContext.debugRect.storageBuffer.reference, guiContext.debugRect.MAX_DEBUG_RECTS * sizeof(_GUIDebugRectInstanceData), 0)
			}))
			}));

		submitResource(manager, &guiContext.debugRect.fragmentShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::FRAGMENT, "vivium4/res/debugrect.frag", "vivium4/res/debugrect_frag.spv")
			}));

		submitResource(manager, &guiContext.debugRect.vertexShader.reference, std::vector<ShaderSpecification>({
			compileShader(ShaderStage::VERTEX, "vivium4/res/debugrect.vert", "vivium4/res/debugrect_vert.spv")
			}));

		submitResource(manager,
			&guiContext.debugRect.pipeline.reference,
			std::vector<PipelineSpecification>({
			PipelineSpecification::fromWindow(
				std::vector<ShaderReference>({ guiContext.debugRect.fragmentShader.reference, guiContext.debugRect.vertexShader.reference }),
				BufferLayout::fromTypes(std::vector<ShaderDataType>({ ShaderDataType::VEC2 })),
				std::vector<DescriptorLayoutReference>({ guiContext.debugRect.descriptorLayout.reference }),
				std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Perspective))}),
				window
			) }));
	}

	GUIElementReference createGUIElement(GUIContext& context)
	{
		GUIElement element;
		element.type = GUIElementType::NONE;
		element.data.arbitrary = ArbitraryUpdateData(nullptr, nullptr);

		context.guiElements.push_back(element);

		return GUIElementReference(context.guiElements.size() - 1);
	}

	GUIElementReference createGUIElement(GUIContext& context, GUIElementType elementType)
	{
		GUIElement element;
		element.type = elementType;
		element.data.arbitrary = ArbitraryUpdateData(nullptr, nullptr);

		context.guiElements.push_back(element);

		return GUIElementReference(context.guiElements.size() - 1);
	}

	GUIElementReference createGUIElement(GUIContext& context, _ContainerUpdateData updateData)
	{
		GUIElement element;
		element.type = GUIElementType::CARDINAL_CONTAINER;
		element.data.container = updateData;

		context.guiElements.push_back(element);

		return GUIElementReference(context.guiElements.size() - 1);
	}

	GUIElementReference defaultGUIParent(GUIContext& context)
	{
		return context.defaultParent;
	}

	GUIElementReference nullGUIParent()
	{
		return GUIElementReference(UINT64_MAX);
	}

	// TODO: create doesn't match the pattern, elements that require a setup, should also be `submit`
	GUIContext createGUIContext(ResourceManager& manager, Engine& engine, Window& window, StitchedAtlas const* spriteAtlas) {
		// TODO: move the code, should be done in some initialisation function
		// Generate the font if it doesn't exist
		if (!std::filesystem::exists("vivium4/res/fonts/consola.sdf"))
		{
			compileSignedDistanceField("vivium4/res/fonts/consola.ttf", 512, "vivium4/res/fonts/consola.sdf", 48, 1.0f);
		}

		GUIContext context;

		context.sprite.atlas = spriteAtlas;
		_submitGenericGUIContext(context, manager, engine, window);
		_submitTextGUIContext(context, manager, engine, window);
		_submitButtonGUIContext(context, manager, engine, window);
		_submitPanelGUIContext(context, manager, engine, window);
		_submitSliderGUIContext(context, manager, engine, window);
		_submitSpriteGUIContext(context, manager, engine, window);
		_submitDebugRectGUIContext(context, manager, engine, window);

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

		convertResourceReference(manager, guiContext.slider.pipeline);
		convertResourceReference(manager, guiContext.slider.descriptorLayout);
		convertResourceReference(manager, guiContext.slider.fragmentShader);
		convertResourceReference(manager, guiContext.slider.vertexShader);
		convertResourceReference(manager, guiContext.slider.storageBuffer);
		convertResourceReference(manager, guiContext.slider.descriptorSet);

		convertResourceReference(manager, guiContext.sprite.pipeline);
		convertResourceReference(manager, guiContext.sprite.descriptorLayout);
		convertResourceReference(manager, guiContext.sprite.texture); // TODO: necessary?
		convertResourceReference(manager, guiContext.sprite.fragmentShader);
		convertResourceReference(manager, guiContext.sprite.vertexShader);
		convertResourceReference(manager, guiContext.sprite.storageBuffer);
		convertResourceReference(manager, guiContext.sprite.descriptorSet);

		convertResourceReference(manager, guiContext.debugRect.pipeline);
		convertResourceReference(manager, guiContext.debugRect.descriptorLayout);
		convertResourceReference(manager, guiContext.debugRect.fragmentShader);
		convertResourceReference(manager, guiContext.debugRect.vertexShader);
		convertResourceReference(manager, guiContext.debugRect.storageBuffer);
		convertResourceReference(manager, guiContext.debugRect.descriptorSet);

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
		// TODO: this is a really old TODO? circumvented this (poorly)
		//	by making a large staging buffer, and using the first n bytes for vertex data
		//  and the last m bytes for index data

		VkDeviceMemory temporaryMemory;
		VkBuffer stagingBuffer;
		void* stagingMapping;
		// Staging buffer both for vertex data and index data
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

		dropShader(guiContext.slider.fragmentShader.resource, engine);
		dropShader(guiContext.slider.vertexShader.resource, engine);

		dropShader(guiContext.sprite.fragmentShader.resource, engine);
		dropShader(guiContext.sprite.vertexShader.resource, engine);

		dropShader(guiContext.debugRect.fragmentShader.resource, engine);
		dropShader(guiContext.debugRect.vertexShader.resource, engine);
		// TODO: maybe the descriptor layout can be freed here?
	}

	void updateGUIContext(GUIContext& guiContext, F32x2 windowDimensions)
	{
		// TODO: update all children of the default parent
		updateGUIElement(defaultGUIParent(guiContext), defaultGUIParent(guiContext), windowDimensions, guiContext);
	}

	void dropGUIContext(GUIContext& guiContext, Engine& engine) {
		guiContext.guiElements = {};

		dropTexture(guiContext.sprite.texture.resource, engine);

		dropDescriptorLayout(guiContext.text.descriptorLayout.resource, engine);
		dropDescriptorLayout(guiContext.button.descriptorLayout.resource, engine);
		dropDescriptorLayout(guiContext.panel.descriptorLayout.resource, engine);
		dropDescriptorLayout(guiContext.slider.descriptorLayout.resource, engine);
		dropDescriptorLayout(guiContext.sprite.descriptorLayout.resource, engine);
		dropDescriptorLayout(guiContext.debugRect.descriptorLayout.resource, engine);
		
		dropPipeline(guiContext.text.pipeline.resource, engine);
		dropPipeline(guiContext.button.pipeline.resource, engine);
		dropPipeline(guiContext.panel.pipeline.resource, engine);
		dropPipeline(guiContext.slider.pipeline.resource, engine);
		dropPipeline(guiContext.sprite.pipeline.resource, engine);
		dropPipeline(guiContext.debugRect.pipeline.resource, engine);

		dropBuffer(guiContext.button.storageBuffer.resource, engine);
		dropBuffer(guiContext.panel.storageBuffer.resource, engine);
		dropBuffer(guiContext.slider.storageBuffer.resource, engine);
		dropBuffer(guiContext.sprite.storageBuffer.resource, engine);
		dropBuffer(guiContext.debugRect.storageBuffer.resource, engine);

		dropBuffer(guiContext.rectVertexBuffer.resource, engine);
		dropBuffer(guiContext.rectIndexBuffer.resource, engine);
	}
}