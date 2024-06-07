#include "scene.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			void renderScene(Scene& scene, Commands::Context::Handle context, GUI::Visual::Context::Handle guiContext, Window::Handle window)
			{
				// TODO: batch together text objects somehow (needs to be on submission)

				std::vector<Context::_ButtonInstanceData> buttonData(scene.buttons.size());
				
				for (uint64_t i = 0; i < scene.buttons.size(); i++) {
					Button::Handle button = scene.buttons[i];

					GUI::Object::update(button, Window::dimensions(window));

					Context::_ButtonInstanceData instance;
					instance.position = button->base->properties.truePosition;
					instance.scale = button->base->properties.trueDimensions;
					instance.foregroundColor = button->color;

					buttonData[i] = instance;
				}

				Math::Perspective perspective = Math::orthogonalPerspective2D(window, F32x2(0.0f), 0.0f, 1.0f);

				setBuffer(guiContext->button.storageBuffer.resource, 0, buttonData.data(), buttonData.size() * sizeof(Context::_ButtonInstanceData));
				Commands::bindPipeline(context, guiContext->button.pipeline.resource);
				Commands::bindVertexBuffer(context, guiContext->button.vertexBuffer.resource);
				Commands::bindIndexBuffer(context, guiContext->button.indexBuffer.resource);
				Commands::bindDescriptorSet(context, guiContext->button.descriptorSet.resource, guiContext->button.pipeline.resource);
				Commands::pushConstants(context, &perspective, sizeof(Math::Perspective), 0, ShaderStage::VERTEX, guiContext->button.pipeline.resource);
				Commands::drawIndexed(context, 6, scene.buttons.size());

				for (Button::Handle button : scene.buttons) {
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
			}
		}
	}
}