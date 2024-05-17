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

				void render(Button::Handle button, Commands::Context::Handle context, Math::Perspective perspective)
				{
					struct UniformData {
						F32x2 position;
						F32x2 scale;
						Color color;
					};

					UniformData uniformData;

					uniformData.position = button->base->properties.truePosition;
					uniformData.scale = button->base->properties.trueDimensions;
					uniformData.color = button->color;

					Buffer::set(button->uniformBuffer, 0, &uniformData, sizeof(uniformData), 0);

					Commands::bindPipeline(context, button->pipeline);
					Commands::bindDescriptorSet(context, button->descriptorSet, button->pipeline);
					Commands::bindIndexBuffer(context, button->deviceIndex);
					Commands::bindVertexBuffer(context, button->deviceVertex);
					Commands::pushConstants(context, &perspective, sizeof(Math::Perspective), 0, Shader::Stage::VERTEX, button->pipeline);
					Commands::drawIndexed(context, 6, 1);

					// TODO: verify more than 0 lines of text
					// Calculate text scale
					float maxLineWidth = std::numeric_limits<float>::lowest();

					for (float lineWidth : button->textMetrics.lineWidths) {
						maxLineWidth = maxLineWidth < lineWidth ? lineWidth : maxLineWidth;
					}

					F32x2 axisScale = button->base->properties.trueDimensions / F32x2(maxLineWidth, button->textMetrics.totalHeight);

					// TODO: pass in text color, or have solution for white text on black
					Text::render(
						button->text,
						context,
						Color::multiply(button->color, 0.4f),
						button->base->properties.truePosition,
						std::min(axisScale.x, axisScale.y),
						perspective
					);
				}
				
				Properties& properties(Button::Handle button)
				{
					return properties(button->base);
				}
				
				void setText(Button::Handle button, Engine::Handle engine, Commands::Context::Handle context, const std::span<const char> text)
				{
					button->textMetrics = Text::calculateMetrics(text.data(), text.size(), button->text->font);
					Text::setText(button->text, engine, button->textMetrics, context, text.data(), text.size(), 1.0f, Text::Alignment::LEFT);
				}
			}
		}
	}
}
