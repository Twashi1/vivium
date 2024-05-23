#include "button.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			namespace Button {
				void setup(Button::Handle button, Commands::Context::Handle context, Engine::Handle engine)
				{
					// Setup vertices and indices
					float vertices[8] = {
						0.0f, 0.0f,
						1.0f, 0.0f,
						1.0f, 1.0f,
						0.0f, 1.0f
					};

					uint16_t indices[6] = { 0, 1, 2, 2, 3, 0 };

					Buffer::set(button->stagingVertex, 0, vertices, sizeof(vertices), 0);
					Buffer::set(button->stagingIndex, 0, indices, sizeof(indices), 0);

					Commands::Context::beginTransfer(context);

					Commands::transferBuffer(context, button->stagingVertex, button->deviceVertex);
					Commands::transferBuffer(context, button->stagingIndex, button->deviceIndex);

					Commands::Context::endTransfer(context, engine);
				}

				void render(Button::Handle button, Commands::Context::Handle context, Context::Handle guiContext, Window::Handle window, Math::Perspective perspective)
				{
					struct UniformData {
						F32x2 position;
						F32x2 scale;
						Color color;
					};

					GUI::Object::update(button, Window::dimensions(window));

					UniformData uniformData;

					uniformData.position = button->base->properties.truePosition;
					uniformData.scale = button->base->properties.trueDimensions;
					uniformData.color = button->color;

					Buffer::set(button->uniformBuffer, 0, &uniformData, sizeof(uniformData), 0);

					Commands::bindPipeline(context, guiContext->button.pipeline);
					Commands::bindDescriptorSet(context, button->descriptorSet, guiContext->button.pipeline);
					Commands::bindIndexBuffer(context, button->deviceIndex);
					Commands::bindVertexBuffer(context, button->deviceVertex);
					Commands::pushConstants(context, &perspective, sizeof(Math::Perspective), 0, Shader::Stage::VERTEX, guiContext->button.pipeline);
					Commands::drawIndexed(context, 6, 1);

					F32x2 axisScale = 0.9f * (button->base->properties.trueDimensions / F32x2(button->textMetrics.maxLineWidth, button->textMetrics.totalHeight));

					// TODO: pass in text color, or have solution for white text on black
					// TODO: text should have auto-updating object base as well
					Text::render(
						button->text,
						button->textMetrics,
						context,
						guiContext,
						Color::multiply(button->color, 0.4f),
						F32x2(std::min(axisScale.x, axisScale.y)),
						perspective
					);
				}
				
				void setText(Button::Handle button, Engine::Handle engine, Window::Handle window, Commands::Context::Handle context, const std::string_view& text)
				{
					// Early exit if no text
					if (text.size() == 0) return;

					button->textMetrics = Text::calculateMetrics(text, button->text->font);

					GUI::Object::update(button, Window::dimensions(window));

					Text::setText(button->text, engine, button->textMetrics, context, text, Text::Alignment::CENTER);
				}
			}
		}
	}
}
