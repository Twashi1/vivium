#include "button.h"

namespace Vivium {
	void dropButton(Button& button, Engine::Handle engine)
	{
		dropText(button.text, engine);
	}

	Button submitButton(ResourceManager::Static::Handle manager, GUI::Visual::Context::Handle guiContext, Engine::Handle engine, Window::Handle window)
	{
		Button button;

		button.base = GUI::Visual::Context::_allocateGUIElement(guiContext);
		button.color = Color::White;

		// TODO: maximum text length should be parameter
		button.text = submitText(manager, engine, guiContext, TextSpecification{ 64, Font::Font::fromDistanceFieldFile("res/fonts/consola.sdf") });
		// TODO: use different method
		_addChild(button.base, { &button.text.base, 1 });
		GUIProperties& textProperties = _properties(button.text.base);
		textProperties.dimensions = F32x2(0.9f);
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
		setupText(button.text, manager);
	}

	void setButtonText(Button& button, Engine::Handle engine, Window::Handle window, Commands::Context::Handle context, const std::string_view& text)
	{
		// Early exit if no text
		if (text.size() == 0) return;

		button.metrics = calculateTextMetrics(text, button.text.font);

		// NOTE: used to be an update to GUI position component here

		setText(button.text, engine, button.metrics, context, text, TextAlignment::CENTER);
	}

	void renderButtons(const std::span<Button*> buttons, Commands::Context::Handle context, GUI::Visual::Context::Handle guiContext, Window::Handle window)
	{
		std::vector<GUI::Visual::Context::_ButtonInstanceData> buttonData(buttons.size());

		for (uint64_t i = 0; i < buttons.size(); i++) {
			Button& button = *buttons[i];

			// TODO: modifies buttons
			// TODO: shouldn't do this, should have some root element that gets updated by user (or automatically)
			_updateGUIElement(button.base, nullptr, Window::dimensions(window));

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
			
			F32x2 axisScale = 0.9f * (button.base->properties.trueDimensions / F32x2(button.metrics.maxLineWidth, button.metrics.totalHeight));

			// TODO: pass in text color, or have solution for white text on black
			renderText(
				button.text,
				button.metrics,
				context,
				guiContext,
				Color::multiply(button.color, 0.4f),
				F32x2(std::min(axisScale.x, axisScale.y)),
				perspective
			);
		}
	}
}