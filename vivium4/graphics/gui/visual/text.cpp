#include "text.h"

namespace Vivium {
	TextMetrics calculateTextMetrics(std::string_view const& text, Font::Font const& font) {
		TextMetrics metrics;

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

	std::vector<PerGlyphData> generateTextRenderData(TextMetrics const& metrics, const std::string_view& text, const Font::Font& font, F32x2 scale, TextAlignment alignment)
	{
		// TODO: investigate how this works with vertical scaling on multiple lines

		std::vector<PerGlyphData> renderData;
		renderData.reserve(metrics.drawableCharacterCount);

		F32x2 position = F32x2(0.0f);

		if (alignment == TextAlignment::CENTER) {
			position.y -= metrics.firstLineHeight * scale.y;
			position.y += metrics.totalHeight * 0.5f * scale.y;
		}

		// TODO: right side alignment
		F32x2 origin = position;

		if (alignment == TextAlignment::CENTER) {
			position.x = -metrics.lineWidths[0] * 0.5f * scale.x;
		}

		uint64_t newLineIndex = 0;

		for (uint64_t i = 0; i < text.size(); i++) {
			char character = text.data()[i];

			if (character == '\n') {
				++newLineIndex;

				position.y -= font.fontSize * scale.y;
				position.x = origin.x;

				if (alignment == TextAlignment::CENTER) {
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

	Text submitText(ResourceManager::Static::Handle manager, Engine::Handle engine, GUI::Visual::Context::Handle guiContext, TextSpecification const& specification) {
		Text text;

		text.base = GUI::Visual::Context::_allocateGUIElement(guiContext);
		// TODO: pass from specification
		text.alignment = TextAlignment::CENTER;

		text.bufferLayout = BufferLayout::fromTypes(std::vector<ShaderDataType>({
			ShaderDataType::VEC2,
			ShaderDataType::VEC2
			}));

		text.batch = submitBatch(engine, manager, BatchSpecification(
			specification.maxCharacterCount * 4,
			specification.maxCharacterCount * 6,
			text.bufferLayout
		));

		std::array<BufferReference, 2> hostBuffers;

		ResourceManager::Static::submit(manager, hostBuffers.data(), MemoryType::UNIFORM, std::vector<BufferSpecification>({
			BufferSpecification(sizeof(Color), BufferUsage::UNIFORM),
			BufferSpecification(sizeof(TextTransformData), BufferUsage::UNIFORM),
			}));

		text.fragmentUniform.reference = hostBuffers[0];
		text.vertexUniform.reference = hostBuffers[1];

		text.font = specification.font;

		ResourceManager::Static::submit(manager, &text.textAtlasTexture.reference, std::vector<TextureSpecification>({
			TextureSpecification::fromFont(specification.font, TextureFormat::MONOCHROME, TextureFilter::NEAREST)
			}));

		ResourceManager::Static::submit(manager, &text.descriptorSet.reference, std::vector<DescriptorSetSpecification>({
			DescriptorSetSpecification(guiContext->text.descriptorLayout.reference, std::vector<UniformData>({
				UniformData::fromTexture(text.textAtlasTexture.reference),
				UniformData::fromBuffer(text.fragmentUniform.reference, sizeof(Color), 0),
				UniformData::fromBuffer(text.vertexUniform.reference, sizeof(F32x2), 0)
				}))
			}));

		return text;
	}

	void renderText(Text& text, TextMetrics const& metrics, Commands::Context::Handle context, GUI::Visual::Context::Handle guiContext, Color color, F32x2 scale, Math::Perspective perspective)
	{
		TextTransformData transform;
		transform.translation = text.base->properties.truePosition;
		transform.scale = scale;
					
		// TODO: test new alignment
		switch (text.alignment) {
		case TextAlignment::LEFT:
			transform.scaleOrigin = F32x2(0.0f, metrics.firstLineHeight);
			break;
		case TextAlignment::CENTER:
			transform.scaleOrigin = F32x2(0.0f);
			break;
		// TODO: right not implemented
		case TextAlignment::RIGHT: break;
		VIVIUM_DEBUG_ONLY(default: VIVIUM_LOG(Log::FATAL, "Invalid alignment passed"); break);
		}

		setBuffer(text.fragmentUniform.resource, 0, &color, sizeof(Color));
		setBuffer(text.vertexUniform.resource, 0, &transform, sizeof(TextTransformData));

		Commands::pushConstants(context, &perspective, sizeof(Math::Perspective), 0, ShaderStage::VERTEX, guiContext->text.pipeline.resource);

		Commands::bindPipeline(context, guiContext->text.pipeline.resource);
		Commands::bindDescriptorSet(context, text.descriptorSet.resource, guiContext->text.pipeline.resource);
		Commands::bindVertexBuffer(context, vertexBufferBatch(text.batch));
		Commands::bindIndexBuffer(context, indexBufferBatch(text.batch));

		Commands::drawIndexed(context, indexCountBatch(text.batch), 1);
	}

	void setText(Text& text, Engine::Handle engine, TextMetrics const& metrics, Commands::Context::Handle context, const std::string_view& textData, TextAlignment alignment)
	{
		// TODO: scale parameter now redundant
		std::vector<PerGlyphData> renderData = generateTextRenderData(metrics, textData, text.font, F32x2(1.0f), alignment);

		uint16_t indices[6] = { 0, 1, 2, 2, 3, 0 };

		for (const PerGlyphData& glyph : renderData) {
			submitRectangleBatch(text.batch, 0, glyph.bottomLeft.x, glyph.bottomLeft.y, glyph.topRight.x, glyph.topRight.y);
			submitRectangleBatch(text.batch, 1, glyph.texBottomLeft.x, glyph.texBottomLeft.y, glyph.texTopRight.x, glyph.texTopRight.y);
			endShapeBatch(text.batch, 4, indices);
		}

		endSubmissionBatch(text.batch, context, engine);
	}

	void dropText(Text& text, Engine::Handle engine) {
		dropBatch(text.batch, engine);

		dropBuffer(text.fragmentUniform.resource, engine);
		dropBuffer(text.vertexUniform.resource, engine);
		dropTexture(text.textAtlasTexture.resource, engine);
	}
				
	void setupText(Text& text, ResourceManager::Static::Handle manager)
	{
		// ugly but fine
		setupBatch(text.batch, manager);

		ResourceManager::Static::convertReference(manager, text.fragmentUniform);
		ResourceManager::Static::convertReference(manager, text.vertexUniform);
		ResourceManager::Static::convertReference(manager, text.textAtlasTexture);
		ResourceManager::Static::convertReference(manager, text.descriptorSet);
	}
}