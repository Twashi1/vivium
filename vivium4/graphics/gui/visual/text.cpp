#include "text.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			namespace Text {
				Metrics calculateMetrics(const std::string_view& text, const Font::Font& font) {
					Metrics metrics;

					metrics.newLineCount = 0;
					metrics.drawableCharacterCount = 0;
					metrics.totalHeight = 0.0f;
					metrics.firstLineHeight = 0.0f;
					metrics.maxLineWidth = 0.0f;

					float currentLineWidth = 0.0f;

					for (uint64_t i = 0; i < text.size(); i++) {
						char character = text.data()[i];

						if (character == '\n') {
							++metrics.newLineCount;

							if (currentLineWidth > metrics.maxLineWidth)
								metrics.maxLineWidth = currentLineWidth;

							metrics.lineWidths.push_back(currentLineWidth);
							currentLineWidth = 0.0f;

							continue;
						}

						Font::Character fontCharacter = font.characters[character];

						currentLineWidth += fontCharacter.advance;

						if (!isspace(character)) ++metrics.drawableCharacterCount;

						if (metrics.newLineCount == 0)
							if (fontCharacter.size.y > metrics.firstLineHeight)
								metrics.firstLineHeight = static_cast<float>(fontCharacter.size.y);
					}

					metrics.lineWidths.push_back(currentLineWidth);
					metrics.totalHeight = font.fontSize * metrics.newLineCount + metrics.firstLineHeight;

					if (currentLineWidth > metrics.maxLineWidth)
						metrics.maxLineWidth = currentLineWidth;

					return metrics;
				}

				std::vector<PerGlyphData> generateRenderData(Metrics metrics, const std::string_view& text, const Font::Font& font, F32x2 scale, Alignment alignment)
				{
					// TODO: investigate how this works with vertical scaling on multiple lines

					std::vector<PerGlyphData> renderData;
					renderData.reserve(metrics.drawableCharacterCount);

					F32x2 position = F32x2(0.0f);

					if (alignment == Alignment::CENTER) {
						position.y -= metrics.firstLineHeight * scale.y;
						position.y += metrics.totalHeight * 0.5f * scale.y;
					}

					// TODO: right side alignment
					F32x2 origin = position;

					if (alignment == Alignment::CENTER) {
						position.x = -metrics.lineWidths[0] * 0.5f * scale.x;
					}

					uint64_t newLineIndex = 0;

					for (uint64_t i = 0; i < text.size(); i++) {
						char character = text.data()[i];

						if (character == '\n') {
							++newLineIndex;

							position.y -= font.fontSize * scale.y;
							position.x = origin.x;

							if (alignment == Alignment::CENTER) {
								position.x += -metrics.lineWidths[newLineIndex] * 0.5f * scale.x;
							}

							continue;
						}

						// TODO: warn on characters we don't know how to draw, or that should never be drawn
						Font::Character fontCharacter = font.characters[character];

						if (!isspace(character)) {
							F32x2 bottomLeft = F32x2(position.x + fontCharacter.bearing.x * scale.x, position.y - (fontCharacter.size.y - fontCharacter.bearing.y) * scale.y);
							F32x2 topRight = bottomLeft + F32x2(static_cast<float>(fontCharacter.size.x), static_cast<float>(fontCharacter.size.y)) * scale;

							// TODO: constructor
							renderData.push_back(PerGlyphData{
								bottomLeft,
								topRight,
								F32x2(fontCharacter.left, fontCharacter.bottom),
								F32x2(fontCharacter.right, fontCharacter.top)
								});
						}

						position.x += fontCharacter.advance * scale.x;
					}

					return renderData;
				}

				void render(Handle handle, Text::Metrics metrics, Commands::Context::Handle context, Context::Handle guiContext, Color color, F32x2 scale, Math::Perspective perspective)
				{
					if (handle->result.indexBuffer == VIVIUM_NULL_HANDLE) return;

					TransformData transform;
					transform.translation = handle->base->properties.truePosition;
					transform.scale = scale;
					
					// TODO: test new alignment
					switch (handle->alignment) {
					case Alignment::LEFT:
						transform.scaleOrigin = F32x2(0.0f, metrics.firstLineHeight);
						break;
					case Alignment::CENTER:
						transform.scaleOrigin = F32x2(0.0f);
						break;
					// TODO: right not implemented
					case Alignment::RIGHT: break;
					VIVIUM_DEBUG_ONLY(default: VIVIUM_LOG(Log::FATAL, "Invalid alignment passed"); break);
					}

					Buffer::set(handle->fragmentUniform, 0, &color, sizeof(Color));
					Buffer::set(handle->vertexUniform, 0, &transform, sizeof(TransformData));

					Commands::pushConstants(context, &perspective, sizeof(Math::Perspective), 0, Shader::Stage::VERTEX, guiContext->text.pipeline);

					Commands::bindPipeline(context, guiContext->text.pipeline);
					Commands::bindDescriptorSet(context, handle->descriptorSet, guiContext->text.pipeline);
					Commands::bindVertexBuffer(context, handle->result.vertexBuffer);
					Commands::bindIndexBuffer(context, handle->result.indexBuffer);

					Commands::drawIndexed(context, handle->result.indexCount, 1);
				}

				void setText(Handle handle, Engine::Handle engine, Metrics metrics, Commands::Context::Handle context, const std::string_view& text, Alignment alignment)
				{
					// TODO: scale parameter now redundant
					std::vector<PerGlyphData> renderData = Text::generateRenderData(metrics, text, handle->font, F32x2(1.0f), alignment);

					uint16_t indices[6] = { 0, 1, 2, 2, 3, 0 };

					for (const PerGlyphData& glyph : renderData) {
						Batch::submitRectangle(handle->batch, 0, glyph.bottomLeft.x, glyph.bottomLeft.y, glyph.topRight.x, glyph.topRight.y);
						Batch::submitRectangle(handle->batch, 1, glyph.texBottomLeft.x, glyph.texBottomLeft.y, glyph.texTopRight.x, glyph.texTopRight.y);
						Batch::endShape(handle->batch, 4, indices);
					}

					handle->result = Batch::endSubmission(handle->batch, context, engine);
				}
			}
		}
	}
}