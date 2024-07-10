#pragma once

#include "../font.h"
#include "../../batch.h"
#include "../../color.h"
#include "../base.h"
#include "context.h"

// TODO: bezier curve mesh-based rendering

namespace Vivium {
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
		float maxLineWidth;
	};

	TextMetrics calculateTextMetrics(std::string_view const& text, Font::Font const& font);
	std::vector<PerGlyphData> generateTextRenderData(TextMetrics const& metrics, const std::string_view& text, const Font::Font& font, F32x2 scale, TextAlignment alignment);

	struct TextSpecification {
		uint64_t maxCharacterCount;
		Font::Font font;
	};

	// TODO: todo outdated?
	// TODO: actually use base property
	struct Text {
		GUIElement* base;

		Batch batch;
		BufferLayout bufferLayout;

		Font::Font font;

		Ref<Buffer> fragmentUniform;
		Ref<Buffer> vertexUniform;
		Ref<Texture> textAtlasTexture;

		Ref<DescriptorSet> descriptorSet;

		TextAlignment alignment;
	};

	void renderText(Text& text, TextMetrics const& metrics, Commands::Context::Handle context, GUI::Visual::Context::Handle guiContext, Color color, F32x2 scale, Math::Perspective perspective);
	void setText(Text& text, Engine::Handle engine, TextMetrics const& metrics, Commands::Context::Handle context, const std::string_view& textData, TextAlignment alignment);

	Text submitText(ResourceManager::Static::Handle manager, Engine::Handle engine, GUI::Visual::Context::Handle textContext, TextSpecification const& specification);

	void setupText(Text& text, ResourceManager::Static::Handle manager);

	void dropText(Text& text, Engine::Handle engine);
}