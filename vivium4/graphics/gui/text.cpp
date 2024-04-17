#include "text.h"

namespace Vivium {
	namespace Text {
		Metrics calculateMetrics(const char* characters, uint64_t length, const Font::Font& font) {
			Metrics metrics;

			metrics.newLineCount = 0;
			metrics.drawableCharacterCount = 0;
			metrics.totalHeight = 0.0f;
			metrics.firstLineHeight = 0.0f;

			float currentLineWidth = 0.0f;

			for (uint64_t i = 0; i < length; i++) {
				char character = characters[i];

				if (character == '\n') {
					++metrics.newLineCount;

					metrics.lineWidths.push_back(currentLineWidth);
					currentLineWidth = 0.0f;

					continue;
				}

				Font::Character fontCharacter = font.characters[character];

				currentLineWidth += fontCharacter.advance;

				if (!isspace(character)) ++metrics.drawableCharacterCount;

				if (metrics.newLineCount == 0)
					if (fontCharacter.size.y > metrics.firstLineHeight)
						metrics.firstLineHeight = fontCharacter.size.y;
			}

			metrics.lineWidths.push_back(currentLineWidth);
			metrics.totalHeight = font.fontSize * metrics.newLineCount + metrics.firstLineHeight;

			return metrics;
		}
		
		std::vector<GlyphInstanceData> generateRenderData(Metrics metrics, const char* characters, uint64_t length, const Font::Font& font, float scale, Alignment alignment)
		{
			std::vector<GlyphInstanceData> renderData;
			renderData.reserve(metrics.drawableCharacterCount);

			F32x2 position = F32x2(0.0f);

			if (alignment == Alignment::CENTER) {
				position.y -= metrics.firstLineHeight * scale;
				position.y += metrics.totalHeight * 0.5f * scale;
			}

			// TODO: right side alignment
			F32x2 origin = position;

			if (alignment == Alignment::CENTER) {
				position.x = -metrics.lineWidths[0] * 0.5f * scale;
			}

			uint64_t newLineIndex = 0;

			for (uint64_t i = 0; i < length; i++) {
				char character = characters[i];

				if (character == '\n') {
					++newLineIndex;

					position.y -= font.fontSize * scale;
					position.x = origin.x;

					if (alignment == Alignment::CENTER) {
						position.x += -metrics.lineWidths[newLineIndex] * 0.5f * scale;
					}

					continue;
				}

				Font::Character fontCharacter = font.characters[character];

				if (!isspace(character)) {
					F32x2 bottomLeft = F32x2(position.x + fontCharacter.bearing.x * scale, position.y - (fontCharacter.size.y - fontCharacter.bearing.y) * scale);
					F32x2 topRight = bottomLeft + F32x2(fontCharacter.size.x, fontCharacter.size.y) * scale;

					// TODO: constructor
					renderData.push_back(GlyphInstanceData{
						bottomLeft,
						topRight,
						F32x2(fontCharacter.left, fontCharacter.bottom),
						F32x2(fontCharacter.right, fontCharacter.top)
					});
				}

				position.x += fontCharacter.advance * scale;
			}

			return renderData;
		}

		void render(Handle handle, Commands::Context::Handle context, Color color, F32x2 position, Math::Perspective perspective)
		{
			Buffer::set(handle->fragmentUniform, 0, &color, sizeof(Color), 0);
			Buffer::set(handle->vertexUniform, 0, &position, sizeof(F32x2), 0);

			Commands::pushConstants(context, &perspective, sizeof(Math::Perspective), 0, Shader::Stage::VERTEX, handle->pipeline);

			Commands::bindPipeline(context, handle->pipeline);
			Commands::bindDescriptorSet(context, handle->descriptorSet, handle->pipeline);
			Commands::bindVertexBuffer(context, handle->result.vertexBuffer);
			Commands::bindIndexBuffer(context, handle->result.indexBuffer);

			Commands::drawIndexed(context, handle->result.indexCount, 1);
		}
		
		void setText(Handle handle, Engine::Handle engine, Metrics metrics, Commands::Context::Handle context, const char* text, uint64_t length, float scale, Alignment alignment)
		{
			std::vector<GlyphInstanceData> renderData = Text::generateRenderData(metrics, text, length, handle->font, scale, alignment);

			uint16_t indices[6] = { 0, 1, 2, 2, 3, 0 };

			for (const GlyphInstanceData& glyph : renderData) {
				// TODO: namespace function
				Batch::submitRectangle(handle->batch, 0, glyph.bottomLeft.x, glyph.bottomLeft.y, glyph.topRight.x, glyph.topRight.y);
				Batch::submitRectangle(handle->batch, 1, glyph.texBottomLeft.x, glyph.texBottomLeft.y, glyph.texTopRight.x, glyph.texTopRight.y);
				Batch::endShape(handle->batch, 4, indices);
			}

			handle->result = Batch::endSubmission(handle->batch, context, engine);
		}
	}
}