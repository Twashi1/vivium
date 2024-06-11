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
					BufferLayout bufferLayout;

					Font::Font font;

					Ref<Buffer> fragmentUniform;
					Ref<Buffer> vertexUniform;
					Ref<Texture> textAtlasTexture;

					Ref<DescriptorSet> descriptorSet;

					Alignment alignment;
				};

				typedef Resource* Handle;
				typedef Resource* PromisedHandle;

				void render(Handle handle, Metrics metrics, Commands::Context::Handle context, Context::Handle guiContext, Color color, F32x2 scale, Math::Perspective perspective);
				void setText(Handle handle, Engine::Handle engine, Metrics metrics, Commands::Context::Handle context, const std::string_view& text, Alignment alignment);

				template <Storage::StorageType StorageType>
				PromisedHandle submit(StorageType* allocator, ResourceManager::Static::Handle manager, Engine::Handle engine, Context::Handle textContext, Specification specification) {
					PromisedHandle handle = Storage::allocateResource<Resource>(allocator);

					handle->base = Object::create(allocator, GUI::Object::Specification());

					handle->bufferLayout = BufferLayout::fromTypes(std::vector<ShaderDataType>({
						ShaderDataType::VEC2,
						ShaderDataType::VEC2
						}));

					handle->batch = Batch::submit(allocator, engine, manager, Batch::Specification(
						specification.maxCharacterCount * 4,
						specification.maxCharacterCount * 6,
						handle->bufferLayout
					));

					std::array<BufferReference, 2> hostBuffers;
					
					ResourceManager::Static::submit(manager, hostBuffers.data(), MemoryType::UNIFORM, std::vector<BufferSpecification>({
						BufferSpecification(sizeof(Color), BufferUsage::UNIFORM),
						BufferSpecification(sizeof(TransformData), BufferUsage::UNIFORM),
					}));

					handle->fragmentUniform.reference = hostBuffers[0];
					handle->vertexUniform.reference = hostBuffers[1];

					handle->font = specification.font;

					ResourceManager::Static::submit(manager, &handle->textAtlasTexture.reference, std::vector<TextureSpecification>({
						TextureSpecification::fromFont(specification.font, TextureFormat::MONOCHROME, TextureFilter::NEAREST)
					}));

					ResourceManager::Static::submit(manager, &handle->descriptorSet.reference, std::vector<DescriptorSetSpecification>({
						DescriptorSetSpecification(textContext->text.descriptorLayout.reference, std::vector<UniformData>({
							UniformData::fromTexture(handle->textAtlasTexture.reference),
							UniformData::fromBuffer(handle->fragmentUniform.reference, sizeof(Color), 0),
							UniformData::fromBuffer(handle->vertexUniform.reference, sizeof(F32x2), 0)
							}))
						}));

					return handle;
				}

				void setup(Text::Handle text, ResourceManager::Static::Handle manager);

				template <Storage::StorageType StorageType>
				void drop(StorageType* allocator, Handle handle, Engine::Handle engine) {
					GUI::Object::drop(allocator, handle->base);

					Batch::drop(allocator, handle->batch, engine);

					dropBuffer(VIVIUM_NULL_STORAGE, handle->fragmentUniform.resource, engine);
					dropBuffer(VIVIUM_NULL_STORAGE, handle->vertexUniform.resource, engine);
					dropTexture(VIVIUM_NULL_STORAGE, handle->textAtlasTexture.resource, engine);

					dropDescriptorSet(allocator, handle->descriptorSet.resource);

					Storage::dropResource(allocator, handle);
				}
			}
		}
	}
}