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
		
		void Resource::submit(uint64_t maxCharacterCount, Allocator::Static::Pool storage, ResourceManager::Static::Handle manager, Engine::Handle engine, const Font::Font& font)
		{
			bufferLayout = Buffer::createLayout(std::vector<Shader::DataType>({
				Shader::DataType::VEC2,
				Shader::DataType::VEC2
				}));

			// TODO: rename in order?
			batch = Batch::create(storage, Batch::Specification(
				maxCharacterCount * 4,
				maxCharacterCount * 6,
				bufferLayout
			), engine, manager);

			std::vector<Buffer::Handle> hostBuffers = ResourceManager::Static::submit(manager, MemoryType::UNIFORM, std::vector<Buffer::Specification>({
				Buffer::Specification(sizeof(TextFragmentUniformData), Buffer::Usage::UNIFORM),
				Buffer::Specification(sizeof(TextVertexUniformData), Buffer::Usage::UNIFORM),
			}));

			fragmentUniform = hostBuffers[0];
			vertexUniform = hostBuffers[1];

			this->font = font;

			std::vector<Texture::Handle> textures = ResourceManager::Static::submit(manager, std::vector<Texture::Specification>({
				Texture::Specification::fromFont(font, Texture::Format::MONOCHROME, Texture::Filter::LINEAR)
				}));

			textAtlasTexture = textures[0];

			descriptorLayout = DescriptorLayout::create(storage, engine, DescriptorLayout::Specification(std::vector<Uniform::Binding>({
				Uniform::Binding(Shader::Stage::FRAGMENT, 0, Uniform::Type::TEXTURE),
				Uniform::Binding(Shader::Stage::FRAGMENT, 1, Uniform::Type::UNIFORM_BUFFER),
				Uniform::Binding(Shader::Stage::VERTEX, 2, Uniform::Type::UNIFORM_BUFFER)
				})));

			matrixPushConstants = Uniform::PushConstant(Shader::Stage::VERTEX, sizeof(Math::Perspective), 0);

			std::vector<DescriptorSet::Handle> descriptorSets = ResourceManager::Static::submit(manager, std::vector<DescriptorSet::Specification>({
				DescriptorSet::Specification(descriptorLayout, std::vector<Uniform::Data>({
					Uniform::Data::fromTexture(textAtlasTexture),
					Uniform::Data::fromBuffer(fragmentUniform, sizeof(TextFragmentUniformData), 0),
					Uniform::Data::fromBuffer(vertexUniform, sizeof(TextVertexUniformData), 0)
					}))
				}));

			descriptorSet = descriptorSets[0];
		}
		
		void Resource::create(Allocator::Static::Pool storage, Window::Handle window, Engine::Handle engine, ResourceManager::Static::Handle manager)
		{
			Shader::Specification fragmentSpecification = Shader::compile(Shader::Stage::FRAGMENT, "testGame/res/text.frag", "testGame/res/text_frag.spv");
			Shader::Specification vertexSpecification = Shader::compile(Shader::Stage::VERTEX, "testGame/res/text.vert", "testGame/res/text_vert.spv");

			Shader::Handle fragment = Shader::create(storage, engine, fragmentSpecification);
			Shader::Handle vertex = Shader::create(storage, engine, vertexSpecification);

			pipeline = Pipeline::create(storage, engine, window, Pipeline::Specification(
				std::vector<Shader::Handle>({ fragment, vertex }),
				bufferLayout,
				std::vector<DescriptorLayout::Handle>({ descriptorLayout }),
				std::vector<Uniform::PushConstant>({ matrixPushConstants })
			));

			Shader::drop(storage, fragment, engine);
			Shader::drop(storage, vertex, engine);
		}
		
		void Resource::setText(Engine::Handle engine, Metrics metrics, Commands::Context::Handle context, const char* text, uint64_t length, float scale, Alignment alignment)
		{
			std::vector<GlyphInstanceData> renderData = Text::generateRenderData(metrics, text, length, font, scale, alignment);

			uint16_t indices[6] = { 0, 1, 2, 2, 3, 0 };

			for (const GlyphInstanceData& glyph : renderData) {
				// TODO: namespace function
				batch->submitRectangle(0, glyph.bottomLeft.x, glyph.bottomLeft.y, glyph.topRight.x, glyph.topRight.y);
				batch->submitRectangle(1, glyph.texBottomLeft.x, glyph.texBottomLeft.y, glyph.texTopRight.x, glyph.texTopRight.y);
				batch->endShape(4, indices, 6, 0);
			}

			result = batch->finish(context, engine);
		}
		
		void Resource::render(TextFragmentUniformData fragmentUniformData, TextVertexUniformData vertexUniformData, Commands::Context::Handle context, Math::Perspective perspective)
		{
			Buffer::set(fragmentUniform, 0, &fragmentUniformData, sizeof(TextFragmentUniformData), 0);
			Buffer::set(vertexUniform, 0, &vertexUniformData, sizeof(TextVertexUniformData), 0);

			Commands::pushConstants(context, &perspective, sizeof(Math::Perspective), 0, Shader::Stage::VERTEX, pipeline);

			Commands::bindPipeline(context, pipeline);
			Commands::bindDescriptorSet(context, descriptorSet, pipeline);
			Commands::bindVertexBuffer(context, result.vertexBuffer);
			Commands::bindIndexBuffer(context, result.indexBuffer);

			Commands::drawIndexed(context, result.indexCount, 1);
		}
		
		void Resource::drop(Allocator::Static::Pool storage, Engine::Handle engine)
		{
			Batch::drop(VIVIUM_RESOURCE_ALLOCATED, batch, engine);

			Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, fragmentUniform, engine);
			Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, vertexUniform, engine);
			Texture::drop(VIVIUM_RESOURCE_ALLOCATED, textAtlasTexture, engine);

			DescriptorLayout::drop(storage, descriptorLayout, engine);
			Pipeline::drop(storage, pipeline, engine);
		}
	}
}