#include "button.h"
#include "context.h"

namespace Vivium {
	void dropButton(Button& button, Engine& engine, GUIContext& guiContext)
	{
		dropTextBatch(button.textBatch, engine);
	}

	Button submitButton(ResourceManager& manager, GUIContext& guiContext, ButtonSpecification specification)
	{
		Button button;

		button.base = createGUIElement(guiContext);
		addChild(specification.parent, { &button.base, 1 }, guiContext);
		button.color = specification.color;
		button.textColor = specification.textColor;

		// TODO: maximum text length should be parameter
		button.textBatch = submitTextBatch(manager, guiContext, TextBatchSpecification{ 64, createFontDistanceField("vivium4/res/fonts/consola.sdf") });
		button.text = createText(TextSpecification{ button.base, "", specification.textColor, calculateTextMetrics("", button.textBatch.font), TextAlignment::CENTER }, guiContext);

		addChild(button.base, { &button.text.base, 1 }, guiContext);
		
		GUIProperties& textProperties = properties(button.text.base, guiContext);
		textProperties.dimensions = F32x2(0.90f);
		textProperties.position = F32x2(0.0f);
		textProperties.unitsType = GUIUnits::RELATIVE;
		textProperties.positionType = GUIPositionType::RELATIVE;
		textProperties.anchorX = GUIAnchor::CENTER;
		textProperties.anchorY = GUIAnchor::CENTER;
		textProperties.centerX = GUIAnchor::LEFT;
		textProperties.centerY = GUIAnchor::BOTTOM;

		return button;
	}

	void setupButton(Button& button, ResourceManager& manager)
	{
		setupTextBatch(button.textBatch, manager);
	}

	void setButtonText(Button& button, Engine& engine, CommandContext& context, GUIContext& guiContext, std::string_view text)
	{
		// Early exit if no text
		if (text.size() == 0) return;

		setText(button.text, calculateTextMetrics(text, button.textBatch.font), text, button.textColor, button.text.alignment);
		
		Text* textObjects[] = { &button.text };
		calculateTextBatch(button.textBatch, textObjects, context, guiContext, engine);
	}

	void submitButtons(std::span<Button*> const buttons, GUIContext& guiContext)
	{
		std::vector<TextBatch*> textBatches;

		for (uint64_t i = 0; i < buttons.size(); i++) {
			Button& button = *buttons[i];

			_GUIButtonInstanceData instance;
			instance.position = properties(button.base, guiContext).truePosition;
			instance.scale = properties(button.base, guiContext).trueDimensions;
			instance.foregroundColor = button.color;

			guiContext.button.buttons.push_back(instance);
			textBatches.push_back(&button.textBatch);
		}

		submitTextBatches(textBatches, guiContext);
	}

	void renderButtons(CommandContext& context, GUIContext& guiContext, Window& window)
	{
		Perspective perspective = orthogonalPerspective2D(windowDimensions(window), F32x2(0.0f), 0.0f, 1.0f);

		setBuffer(guiContext.button.storageBuffer.resource, 0, guiContext.button.buttons.data(), guiContext.button.buttons.size() * sizeof(_GUIButtonInstanceData));
		cmdBindPipeline(context, guiContext.button.pipeline.resource);
		cmdBindVertexBuffer(context, guiContext.rectVertexBuffer.resource);
		cmdBindIndexBuffer(context, guiContext.rectIndexBuffer.resource);
		cmdBindDescriptorSet(context, guiContext.button.descriptorSet.resource, guiContext.button.pipeline.resource);
		cmdWritePushConstants(context, &perspective, sizeof(Perspective), 0, ShaderStage::VERTEX, guiContext.button.pipeline.resource);
		cmdDrawIndexed(context, 6, guiContext.button.buttons.size());

		guiContext.button.buttons.clear();
	}
}