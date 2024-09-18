#include "button.h"

namespace Vivium {
	void dropButton(Button& button, Engine::Handle engine, GUIContext& guiContext)
	{
		dropText(button.text, guiContext);
		dropTextBatch(button.textBatch, engine);
	}

	Button submitButton(ResourceManager::Static::Handle manager, GUIContext& guiContext, Engine::Handle engine, Window::Handle window, ButtonSpecification specification)
	{
		Button button;

		button.base = createGUIElement(guiContext);
		addChild(specification.parent, { &button.base, 1 }, guiContext);
		button.color = specification.color;
		button.textColor = specification.textColor;

		// TODO: maximum text length should be parameter
		button.textBatch = submitTextBatch(manager, engine, guiContext, TextBatchSpecification{ 64, Font::Font::fromDistanceFieldFile("vivium4/res/fonts/consola.sdf") });
		button.text = createText(TextSpecification{ button.base, "", specification.textColor, calculateTextMetrics("", button.textBatch.font), TextAlignment::CENTER }, guiContext);

		addChild(button.base, { &button.text.base, 1 }, guiContext);
		
		GUIProperties& textProperties = properties(button.text.base, guiContext);
		textProperties.dimensions = F32x2(0.95f);
		textProperties.position = F32x2(0.0f);
		textProperties.unitsType = GUIUnits::RELATIVE;
		textProperties.positionType = GUIPositionType::RELATIVE;
		textProperties.anchorX = GUIAnchor::CENTER;
		textProperties.anchorY = GUIAnchor::CENTER;
		textProperties.centerX = GUIAnchor::LEFT;
		textProperties.centerY = GUIAnchor::BOTTOM;

		return button;
	}

	void setupButton(Button& button, ResourceManager::Static::Handle manager)
	{
		setupTextBatch(button.textBatch, manager);
	}

	void setButtonText(Button& button, Engine::Handle engine, Window::Handle window, Commands::Context::Handle context, GUIContext& guiContext, std::string_view text)
	{
		// Early exit if no text
		if (text.size() == 0) return;

		setText(button.text, calculateTextMetrics(text, button.textBatch.font), text, button.textColor, button.text.alignment);
		
		Text* textObjects[] = { &button.text };
		calculateTextBatch(button.textBatch, textObjects, context, guiContext, engine);
	}

	void renderButtons(const std::span<Button*> buttons, Commands::Context::Handle context, GUIContext& guiContext, Window::Handle window)
	{
		std::vector<_GUIButtonInstanceData> buttonData(buttons.size());

		for (uint64_t i = 0; i < buttons.size(); i++) {
			Button& button = *buttons[i];

			_GUIButtonInstanceData instance;
			instance.position = properties(button.base, guiContext).truePosition;
			instance.scale = properties(button.base, guiContext).trueDimensions;
			instance.foregroundColor = button.color;

			buttonData[i] = instance;
		}

		Math::Perspective perspective = Math::orthogonalPerspective2D(Window::dimensions(window), F32x2(0.0f), 0.0f, 1.0f);

		setBuffer(guiContext.button.storageBuffer.resource, 0, buttonData.data(), buttonData.size() * sizeof(_GUIButtonInstanceData));
		Commands::bindPipeline(context, guiContext.button.pipeline.resource);
		Commands::bindVertexBuffer(context, guiContext.rectVertexBuffer.resource);
		Commands::bindIndexBuffer(context, guiContext.rectIndexBuffer.resource);
		Commands::bindDescriptorSet(context, guiContext.button.descriptorSet.resource, guiContext.button.pipeline.resource);
		Commands::pushConstants(context, &perspective, sizeof(Math::Perspective), 0, ShaderStage::VERTEX, guiContext.button.pipeline.resource);
		Commands::drawIndexed(context, 6, buttons.size());

		for (Button* buttonPtr : buttons) {
			Button& button = *buttonPtr;
			
			renderTextBatch(button.textBatch, context, guiContext, perspective);
		}
	}
}