#pragma once

#include "../font.h"
#include "../../batch.h"
#include "../../color.h"
#include "../base.h"

// TODO: bezier curve mesh-based rendering

namespace Vivium {
	struct GUIContext;

	enum class TextAlignment {
		LEFT,
		CENTER,
		RIGHT
	};

	struct PerGlyphData {
		F32x2 bottomLeft;
		F32x2 topRight;

		F32x2 texBottomLeft;
		F32x2 texTopRight;
	};

	struct TextTransformData {
		F32x2 translation;
		F32x2 scale;
		F32x2 scaleOrigin;
	};

	struct TextMetrics {
		std::vector<float> lineWidths;
		uint32_t drawableCharacterCount;
		uint32_t newLineCount;

		float firstLineHeight;
		float totalHeight;
		// Including characters that dip below the last line
		float totalHeightAndBottom;
		float maxLineWidth;
	};

	TextMetrics calculateTextMetrics(std::string_view const& text, Font const& font);
	std::vector<PerGlyphData> generateTextRenderData(TextMetrics const& metrics, const std::string_view& text, const Font& font, F32x2 scale, TextAlignment alignment);

	struct TextSpecification {
		GUIElementReference parent;

		std::string characters;
		Color color;
		TextMetrics metrics;
		TextAlignment alignment;
	};

	struct Text {
		GUIElementReference base;

		std::string characters;
		Color color;
		TextMetrics metrics;
		TextAlignment alignment;
	};

	struct TextBatchSpecification {
		uint64_t maxCharacterCount;
		Font font;
	};

	struct TextBatch {
		Batch batch;

		Font font;
		Ref<Texture> fontTexture;
		Ref<DescriptorSet> descriptorSet;
	};

	void submitTextBatches(std::span<TextBatch*> const textBatches, GUIContext& guiContext);
	void renderTextBatch(CommandContext& context, GUIContext& guiContext, Window& window);
	void renderTextBatch(TextBatch& text, CommandContext& context, GUIContext& guiContext, Perspective const& perspective);
	void calculateTextBatch(TextBatch& text, std::span<Text*> textObjects, CommandContext& context, GUIContext& guiContext, Engine& engine);

	TextBatch submitTextBatch(ResourceManager& manager, GUIContext& guiContext, TextBatchSpecification const& specification);
	void setupTextBatch(TextBatch& text, ResourceManager& manager);

	void setText(Text& text, TextMetrics const& metrics, const std::string_view& textData, Color color, TextAlignment alignment);

	Text createText(TextSpecification const& specification, GUIContext& guiContext);

	void dropTextBatch(TextBatch& text, Engine& engine);
}