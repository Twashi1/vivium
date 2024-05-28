#pragma once

#include "../font.h"
#include "../../batch.h"
#include "../../color.h"
#include "../base.h"
#include "context.h"

// TODO: bezier curve mesh-based rendering

namespace Vivium {
	namespace GUI {
		namespace Visual {
			namespace Text {
				enum class Alignment {
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

				struct TransformData {
					F32x2 translation;
					F32x2 scale;
					F32x2 scaleOrigin;
				};

				struct Metrics {
					std::vector<float> lineWidths;
					uint32_t drawableCharacterCount;
					uint32_t newLineCount;

					float firstLineHeight;
					float totalHeight;
					float maxLineWidth;
				};

				Metrics calculateMetrics(const std::string_view& text, const Font::Font& font);
				std::vector<PerGlyphData> generateRenderData(Metrics metrics, const std::string_view& text, const Font::Font& font, F32x2 scale, Alignment alignment);

				struct Specification {
					uint64_t maxCharacterCount;
					Font::Font font;
				};

				// TODO: actually use base property
				struct Resource {
					Object::Handle base;

					Batch::Handle batch;
					Batch::Result result;
					Buffer::Layout bufferLayout;

					Font::Font font;

					Buffer::Handle fragmentUniform;
					Buffer::Handle vertexUniform;
					Texture::Handle textAtlasTexture;

					DescriptorSet::Handle descriptorSet;

					Alignment alignment;
				};

				typedef Resource* Handle;
				typedef Resource* PromisedHandle;

				void render(Handle handle, Metrics metrics, Commands::Context::Handle context, Context::Handle guiContext, Color color, F32x2 scale, Math::Perspective perspective);
				void setText(Handle handle, Engine::Handle engine, Metrics metrics, Commands::Context::Handle context, const std::string_view& text, Alignment alignment);

				template <Allocator::AllocatorType AllocatorType>
				PromisedHandle submit(AllocatorType* allocator, ResourceManager::Static::Handle manager, Engine::Handle engine, Context::Handle textContext, Specification specification) {
					VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);

					PromisedHandle handle = Allocator::allocateResource<Resource>(allocator);

					handle->base = Object::create(allocator, GUI::Object::Specification());

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
						Buffer::Specification(sizeof(TransformData), Buffer::Usage::UNIFORM),
						}));

					handle->fragmentUniform = hostBuffers[0];
					handle->vertexUniform = hostBuffers[1];

					handle->font = specification.font;

					std::vector<Texture::Handle> textures = ResourceManager::Static::submit(manager, std::vector<Texture::Specification>({
						Texture::Specification::fromFont(specification.font, Texture::Format::MONOCHROME, Texture::Filter::NEAREST)
						}));

					handle->textAtlasTexture = textures[0];

					std::vector<DescriptorSet::Handle> descriptorSets = ResourceManager::Static::submit(manager, std::vector<DescriptorSet::Specification>({
						DescriptorSet::Specification(textContext->text.descriptorLayout, std::vector<Uniform::Data>({
							Uniform::Data::fromTexture(handle->textAtlasTexture),
							Uniform::Data::fromBuffer(handle->fragmentUniform, sizeof(Color), 0),
							Uniform::Data::fromBuffer(handle->vertexUniform, sizeof(F32x2), 0)
							}))
						}));

					handle->descriptorSet = descriptorSets[0];

					return handle;
				}

				template <Allocator::AllocatorType AllocatorType>
				void drop(AllocatorType* allocator, Handle handle, Engine::Handle engine) {
					VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);
					VIVIUM_CHECK_HANDLE_EXISTS(handle);

					GUI::Object::drop(allocator, handle->base);

					Batch::drop(allocator, handle->batch, engine);

					Buffer::drop(VIVIUM_NULL_ALLOCATOR, handle->fragmentUniform, engine);
					Buffer::drop(VIVIUM_NULL_ALLOCATOR, handle->vertexUniform, engine);
					Texture::drop(VIVIUM_NULL_ALLOCATOR, handle->textAtlasTexture, engine);

					DescriptorSet::drop(allocator, handle->descriptorSet);

					Allocator::dropResource(allocator, handle);
				}
			}
		}
	}
}