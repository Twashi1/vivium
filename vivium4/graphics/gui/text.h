#pragma once

#include "../primitives/buffer.h"
#include "font.h"
#include "../batch.h"
#include "../color.h"

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

		struct Specification {
			uint64_t maxCharacterCount;
			Font::Font font;
		};

		// TODO: derive GUI trait (Base)
		// TODO: use base for positioning
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
			Shader::Handle fragmentShader;
			Shader::Handle vertexShader;

			DescriptorLayout::Handle descriptorLayout;
			Pipeline::Handle pipeline;
			Uniform::PushConstant matrixPushConstants;
		};

		typedef Resource* Handle;
		typedef Resource* PromisedHandle;

		void render(Handle handle, Commands::Context::Handle context, Color color, F32x2 position, float scale, Math::Perspective perspective);
		void setText(Handle handle, Engine::Handle engine, Metrics metrics, Commands::Context::Handle context, const char* text, uint64_t length, float scale, Alignment alignment);

		template <Allocator::AllocatorType AllocatorType>
		PromisedHandle submit(AllocatorType allocator, ResourceManager::Static::Handle manager, Engine::Handle engine, Specification specification) {
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);

			// TODO: duplicated across multiple locations
			struct VertexUniform {
				F32x2 translation;
				float scale;
			};

			PromisedHandle handle = Allocator::allocateResource<Resource>(allocator);

			handle->bufferLayout = Buffer::Layout::fromTypes(std::vector<Shader::DataType>({
				Shader::DataType::VEC2,
				Shader::DataType::VEC2
				}));

			handle->batch = Batch::submit(allocator, engine, manager, Batch::Specification(
				specification.maxCharacterCount * 4,
				specification.maxCharacterCount * 6,
				handle->bufferLayout
			));

			std::vector<Buffer::Handle> hostBuffers = ResourceManager::Static::submit(manager, MemoryType::UNIFORM, std::vector<Buffer::Specification>({
				Buffer::Specification(sizeof(Color), Buffer::Usage::UNIFORM),
				Buffer::Specification(sizeof(VertexUniform), Buffer::Usage::UNIFORM),
				}));

			handle->fragmentUniform = hostBuffers[0];
			handle->vertexUniform = hostBuffers[1];

			handle->font = specification.font;

			// TODO: for whatever reason, we can't linearly interpolate Format::MONOCHROME (on this system at least)?
			std::vector<Texture::Handle> textures = ResourceManager::Static::submit(manager, std::vector<Texture::Specification>({
				Texture::Specification::fromFont(specification.font, Texture::Format::MONOCHROME, Texture::Filter::NEAREST)
			}));

			handle->textAtlasTexture = textures[0];

			handle->descriptorLayout = DescriptorLayout::create(allocator, engine, DescriptorLayout::Specification(std::vector<Uniform::Binding>({
				Uniform::Binding(Shader::Stage::FRAGMENT, 0, Uniform::Type::TEXTURE),
				Uniform::Binding(Shader::Stage::FRAGMENT, 1, Uniform::Type::UNIFORM_BUFFER),
				Uniform::Binding(Shader::Stage::VERTEX, 2, Uniform::Type::UNIFORM_BUFFER)
				})));

			handle->matrixPushConstants = Uniform::PushConstant(Shader::Stage::VERTEX, sizeof(Math::Perspective), 0);

			std::vector<DescriptorSet::Handle> descriptorSets = ResourceManager::Static::submit(manager, std::vector<DescriptorSet::Specification>({
				DescriptorSet::Specification(handle->descriptorLayout, std::vector<Uniform::Data>({
					Uniform::Data::fromTexture(handle->textAtlasTexture),
					Uniform::Data::fromBuffer(handle->fragmentUniform, sizeof(Color), 0),
					Uniform::Data::fromBuffer(handle->vertexUniform, sizeof(F32x2), 0)
					}))
				}));

			handle->descriptorSet = descriptorSets[0];

			Shader::Specification fragmentSpecification = Shader::compile(Shader::Stage::FRAGMENT, "testGame/res/text.frag", "testGame/res/text_frag.spv");
			Shader::Specification vertexSpecification = Shader::compile(Shader::Stage::VERTEX, "testGame/res/text.vert", "testGame/res/text_vert.spv");

			handle->fragmentShader = Shader::create(allocator, engine, fragmentSpecification);
			handle->vertexShader = Shader::create(allocator, engine, vertexSpecification);

			// TODO: memory lifetime scary here
			std::vector<Pipeline::Handle> pipelines = ResourceManager::Static::submit(manager,
				std::vector<Pipeline::Specification>({ Pipeline::Specification(
					std::vector<Shader::Handle>({ handle->fragmentShader, handle->vertexShader }),
					handle->bufferLayout,
					std::vector<DescriptorLayout::Handle>({ handle->descriptorLayout }),
					std::vector<Uniform::PushConstant>({ handle->matrixPushConstants })
				) }));

			handle->pipeline = pipelines[0];

			return handle;
		}

		template <Allocator::AllocatorType AllocatorType>
		void drop(AllocatorType allocator, Handle handle, Engine::Handle engine) {
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);
			
			Batch::drop(VIVIUM_RESOURCE_ALLOCATED, handle->batch, engine);

			Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, handle->fragmentUniform, engine);
			Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, handle->vertexUniform, engine);
			Texture::drop(VIVIUM_RESOURCE_ALLOCATED, handle->textAtlasTexture, engine);

			Shader::drop(allocator, handle->fragmentShader, engine);
			Shader::drop(allocator, handle->vertexShader, engine);

			DescriptorLayout::drop(allocator, handle->descriptorLayout, engine);
			DescriptorSet::drop(allocator, handle->descriptorSet);
			Pipeline::drop(allocator, handle->pipeline, engine);

			Allocator::dropResource(allocator, handle);
		}
	}
}