#include "button.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			namespace Button {
				void render(Button::Handle button, Commands::Context::Handle context, Math::Perspective perspective)
				{
					struct UniformData {
						Color color;
						F32x2 position;
					};

					UniformData uniformData;

					uniformData.position = F32x2(100.0f);
					uniformData.color = Color::White;

					Buffer::set(button->uniformBuffer, 0, &uniformData, sizeof(uniformData), 0);

					Commands::bindPipeline(context, button->pipeline);
					Commands::bindDescriptorSet(context, button->descriptorSet, button->pipeline);
					Commands::bindIndexBuffer(context, button->deviceIndex);
					Commands::bindVertexBuffer(context, button->deviceVertex);
					Commands::pushConstants(context, &perspective, sizeof(Math::Perspective), 0, Shader::Stage::VERTEX, button->pipeline);
					Commands::drawIndexed(context, 6, 1);

					Text::render(button->text, context, Color::Gray, F32x2(100.0f), perspective);
				}
			}
		}
	}
}
