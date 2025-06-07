#include "text.h"

namespace Vivium {
	TextMetrics calculateTextMetrics(std::string_view const& text, Font::Font const& font) {
		TextMetrics metrics;

		metrics.newLineCount = 0;
		metrics.drawableCharacterCount = 0;
		metrics.totalHeight = 0.0f;
		metrics.firstLineHeight = 0.0f;
		metrics.maxLineWidth = 0.0f;
		metrics.totalHeightAndBottom = 0.0f;

		float currentLineWidth = 0.0f;
		float belowLineSize = 0.0f;

		for (uint64_t i = 0; i < text.size(); i++) {
			char character = text.data()[i];

			if (character == '\n') {
				++metrics.newLineCount;

				if (currentLineWidth > metrics.maxLineWidth)
					metrics.maxLineWidth = currentLineWidth;

				if (belowLineSize > metrics.totalHeightAndBottom)
					metrics.totalHeightAndBottom = belowLineSize;
				
				metrics.lineWidths.push_back(currentLineWidth);
				currentLineWidth = 0.0f;
				belowLineSize = 0.0f;

				continue;
			}

			Font::Character fontCharacter = font.characters[character];

			currentLineWidth += fontCharacter.advance;
			
			float currentBelowLine = fontCharacter.size.y - fontCharacter.bearing.y;

			if (currentBelowLine > belowLineSize)
				belowLineSize = currentBelowLine;

			if (!isspace(character)) ++metrics.drawableCharacterCount;

			if (metrics.newLineCount == 0)
				if (fontCharacter.size.y > metrics.firstLineHeight)
					metrics.firstLineHeight = static_cast<float>(fontCharacter.size.y);
		}

		metrics.lineWidths.push_back(currentLineWidth);
		metrics.totalHeight = font.fontSize * metrics.newLineCount + metrics.firstLineHeight;
		metrics.totalHeightAndBottom = metrics.totalHeight + belowLineSize;

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
			position.y += metrics.totalHeightAndBottom * 0.5f * scale.y;
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

	void renderTextBatch(TextBatch& text, CommandContext& context, GUIContext& guiContext, Math::Perspective const& perspective)
	{
		if (indexCountBatch(text.batch) == 0) { return; }

		cmdWritePushConstants(context, &perspective, sizeof(Math::Perspective), 0, ShaderStage::VERTEX, guiContext.text.pipeline.resource);

		cmdBindPipeline(context, guiContext.text.pipeline.resource);
		cmdBindDescriptorSet(context, text.descriptorSet.resource, guiContext.text.pipeline.resource);
		cmdBindVertexBuffer(context, vertexBufferBatch(text.batch));
		cmdBindIndexBuffer(context, indexBufferBatch(text.batch));

		cmdDrawIndexed(context, indexCountBatch(text.batch), 1);
	}

	void calculateTextBatch(TextBatch& textBatch, std::span<Text*> textObjects, CommandContext& context, GUIContext& guiContext, Engine& engine)
	{
		if (textObjects.size() == 0) { return; }

		uint16_t indices[6] = { 0, 1, 2, 2, 3, 0 };

		for (Text* text : textObjects) {
			// Calculate required scaling and offsets
			GUIProperties const& props = properties(text->base, guiContext);
			F32x2 translation = props.truePosition;
			// Calculate scale to fit to dimensions
			F32x2 axisScale = props.trueDimensions / F32x2(text->metrics.maxLineWidth, text->metrics.totalHeightAndBottom);
			float scale = std::min(axisScale.x, axisScale.y);
			
			// Calculate origin point about which to scale
			F32x2 scaleOrigin;

			switch (text->alignment) {
			case TextAlignment::LEFT:
				scaleOrigin = F32x2(0.0f, text->metrics.firstLineHeight) * scale;
				break;
			case TextAlignment::CENTER:
				scaleOrigin = F32x2(0.0f); break;
			case TextAlignment::RIGHT:
				VIVIUM_LOG(Log::FATAL, "Right alignment not implemented"); break;
			default: VIVIUM_LOG(Log::FATAL, "Invalid alignment"); break;
			}

			std::vector<PerGlyphData> renderData = generateTextRenderData(text->metrics, text->characters, textBatch.font, F32x2(1.0f), text->alignment);

			// Duplicated for each vertex
			Color textColorData[4];
			textColorData[0] = text->color;
			textColorData[1] = text->color;
			textColorData[2] = text->color;
			textColorData[3] = text->color;

			for (PerGlyphData const& glyph : renderData) {
				F32x2 bottomLeft = (glyph.bottomLeft - scaleOrigin) * scale + scaleOrigin + translation;
				F32x2 topRight = (glyph.topRight - scaleOrigin) * scale + scaleOrigin + translation;

				submitRectangleBatch(textBatch.batch, 0, bottomLeft.x, bottomLeft.y, topRight.x, topRight.y);
				submitRectangleBatch(textBatch.batch, 1, glyph.texBottomLeft.x, glyph.texBottomLeft.y, glyph.texTopRight.x, glyph.texTopRight.y);
				submitElementBatch(textBatch.batch, 2, { reinterpret_cast<uint8_t*>(textColorData), sizeof(Color) * 4});
				endShapeBatch(textBatch.batch, 4, indices);
			}
		}
		
		endSubmissionBatch(textBatch.batch, context, engine);
	}

	TextBatch submitTextBatch(ResourceManager& manager, Engine& engine, GUIContext& guiContext, TextBatchSpecification const& specification)
	{
		TextBatch text;

		text.batch = submitBatch(engine, manager, BatchSpecification(
			specification.maxCharacterCount * 4,
			specification.maxCharacterCount * 6,
			guiContext.text.bufferLayout
		));

		text.font = specification.font;

		submitResource(manager, &text.fontTexture.reference, std::vector<TextureSpecification>({
			TextureSpecification::fromFont(specification.font, TextureFormat::MONOCHROME, TextureFilter::NEAREST)
			}));

		submitResource(manager, &text.descriptorSet.reference, std::vector<DescriptorSetSpecification>({
			DescriptorSetSpecification(guiContext.text.descriptorLayout.reference, std::vector<UniformData>({
				UniformData::fromTexture(text.fontTexture.reference)
				}))
			}));

		return text;
	}

	void setupTextBatch(TextBatch& text, ResourceManager& manager)
	{
		setupBatch(text.batch, manager);

		convertResourceReference(manager, text.fontTexture);
		convertResourceReference(manager, text.descriptorSet);
	}

	void setText(Text& text, TextMetrics const& metrics, const std::string_view& textData, Color color, TextAlignment alignment)
	{
		text.metrics = metrics;
		text.characters = textData;
		text.color = color;
		text.alignment = alignment;
	}

	Text createText(TextSpecification const& specification, GUIContext& guiContext)
	{
		GUIElementReference base = createGUIElement(guiContext, GUIElementType::DEFAULT);
		addChild(specification.parent, { &base, 1 }, guiContext);

		return Text{ base, specification.characters, specification.color, specification.metrics, specification.alignment };
	}

	void dropTextBatch(TextBatch& text, Engine& engine)
	{
		dropBatch(text.batch, engine);
		dropTexture(text.fontTexture.resource, engine);
	}
}