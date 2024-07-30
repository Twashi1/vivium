#include "button.h"

namespace Vivium {
	void dropButton(Button& button, Engine::Handle engine)
	{
		dropTextBatch(button.textBatch, engine);
	}

	Button submitButton(ResourceManager::Static::Handle manager, GUI::Visual::Context::Handle guiContext, Engine::Handle engine, Window::Handle window, ButtonSpecification specification)
	{
		Button button;

		if (specification.parent == nullptr) {
			specification.parent = guiContext->defaultParent;
		}

		button.base = GUI::Visual::Context::_allocateGUIElement(guiContext);
		addChild(specification.parent, button.base);
		button.color = specification.color;
		button.textColor = specification.textColor;

		// TODO: maximum text length should be parameter
		button.textBatch = submitTextBatch(manager, engine, guiContext, TextBatchSpecification{ 64, Font::Font::fromDistanceFieldFile("vivium4/res/fonts/consola.sdf") });
		button.text = createText(TextSpecification{ button.base, "", specification.textColor, calculateTextMetrics("", button.textBatch.font), TextAlignment::CENTER }, guiContext);
		properties(button.text)->dimensions = F32x2(0.9f);

		addChild(button.base, button.text.base);
		
		GUIProperties& textProperties = _properties(button.text.base);
		textProperties.dimensions = F32x2(0.95f);
		textProperties.position = F32x2(0.0f);
		textProperties.scaleType = GUIScaleType::RELATIVE;
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

	void setButtonText(Button& button, Engine::Handle engine, Window::Handle window, Commands::Context::Handle context, const std::string_view& text)
	{
		// Early exit if no text
		if (text.size() == 0) return;

		setText(button.text, calculateTextMetrics(text, button.textBatch.font), text, button.textColor, button.text.alignment);
		
		Text* textObjects[] = { &button.text };
		calculateTextBatch(button.textBatch, textObjects, context, engine);
	}

	void renderButtons(const std::span<Button*> buttons, Commands::Context::Handle context, GUI::Visual::Context::Handle guiContext, Window::Handle window)
	{
		std::vector<GUI::Visual::Context::_ButtonInstanceData> buttonData(buttons.size());

		constexpr int x = sizeof(GUI::Visual::Context::_ButtonInstanceData);

		for (uint64_t i = 0; i < buttons.size(); i++) {
			Button& button = *buttons[i];

			GUI::Visual::Context::_ButtonInstanceData instance;
			instance.position = button.base->properties.truePosition;
			instance.scale = button.base->properties.trueDimensions;
			instance.foregroundColor = button.color;

			buttonData[i] = instance;
		}

		Math::Perspective perspective = Math::orthogonalPerspective2D(window, F32x2(0.0f), 0.0f, 1.0f);

		setBuffer(guiContext->button.storageBuffer.resource, 0, buttonData.data(), buttonData.size() * sizeof(GUI::Visual::Context::_ButtonInstanceData));
		Commands::bindPipeline(context, guiContext->button.pipeline.resource);
		Commands::bindVertexBuffer(context, guiContext->rectVertexBuffer.resource);
		Commands::bindIndexBuffer(context, guiContext->rectIndexBuffer.resource);
		Commands::bindDescriptorSet(context, guiContext->button.descriptorSet.resource, guiContext->button.pipeline.resource);
		Commands::pushConstants(context, &perspective, sizeof(Math::Perspective), 0, ShaderStage::VERTEX, guiContext->button.pipeline.resource);
		Commands::drawIndexed(context, 6, buttons.size());

		for (Button* buttonPtr : buttons) {
			Button& button = *buttonPtr;
			
			renderTextBatch(button.textBatch, context, guiContext, perspective);
		}
	}
}