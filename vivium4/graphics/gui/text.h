#pragma once

#include "../primitives/buffer.h"
#include "font.h"
#include "../batch.h"

// TODO: text doesn't exactly work like a GUI object
//		it is close to a GUI visual maybe

// TODO: bezier curve mesh-based rendering

namespace Vivium {
	namespace Text {
		enum class Alignment {
			LEFT,
			CENTER,
			RIGHT
		};

		// TODO: name
		struct GlyphInstanceData {
			F32x2 bottomLeft;
			F32x2 topRight;

			F32x2 texBottomLeft;
			F32x2 texTopRight;
		};

		struct Metrics {
			std::vector<float> lineWidths;
			uint32_t drawableCharacterCount;
			uint32_t newLineCount;

			float firstLineHeight;
			float totalHeight;
		};

		Metrics calculateMetrics(const char* characters, uint64_t length, const Font::Font& font);
		std::vector<GlyphInstanceData> generateRenderData(Metrics metrics, const char* characters, uint64_t length, const Font::Font& font, float scale, Alignment alignment);

		struct TextFragmentUniformData {
			float r, g, b;
		};

		struct TextVertexUniformData {
			F32x2 translation;
		};

		struct Resource {
			Batch::Handle batch;
			Batch::Result result;
			Buffer::Layout bufferLayout;

			Font::Font font;

			Buffer::Handle fragmentUniform;
			Buffer::Handle vertexUniform;
			Texture::Handle textAtlasTexture;

			DescriptorSet::Handle descriptorSet;

			// TODO: should all be statics
			DescriptorLayout::Handle descriptorLayout;
			Pipeline::Handle pipeline;
			Uniform::PushConstant matrixPushConstants;

			void submit(uint64_t maxCharacterCount, Allocator::Static::Pool storage, ResourceManager::Static::Handle manager, Engine::Handle engine, const Font::Font& font);
			void create(Allocator::Static::Pool storage, Window::Handle window, Engine::Handle engine, ResourceManager::Static::Handle manager);

			void setText(Engine::Handle engine, Metrics metrics, Commands::Context::Handle context, const char* text, uint64_t length, float scale, Alignment alignment);
			
			void render(TextFragmentUniformData fragmentUniformData, TextVertexUniformData vertexUniformData, Commands::Context::Handle context, Math::Perspective perspective);

			void drop(Allocator::Static::Pool storage, Engine::Handle engine);
		};

		typedef Resource* Handle;
	}
}