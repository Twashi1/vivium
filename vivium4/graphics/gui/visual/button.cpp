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

					// TODO: position from button base
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

					Text::render(button->text, context, Color::multiply(button->color, 0.4f), F32x2(100.0f), perspective);
				}
				
				Properties& properties(Button::Handle button)
				{
					return properties(button->base);
				}
				
				void setText(Button::Handle button, Engine::Handle engine, Commands::Context::Handle context, const char* text)
				{
					Text::setText(button->text, engine, Text::calculateMetrics(text, strlen(text), button->text->font), context, text, strlen(text), 1.0f, Text::Alignment::LEFT);
				}
			}
		}
	}
}
