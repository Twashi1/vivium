#pragma once

#include <functional>

#include "../engine.h"
#include "primitives/buffer.h"
#include "primitives/memory_type.h"
#include "primitives/texture.h"
#include "primitives/pipeline.h"
#include "primitives/descriptor_layout.h"
#include "primitives/descriptor_set.h"

namespace Vivium {
	namespace Commands {
		void createBuffer(Engine::Handle engine, VkBuffer* buffer, uint64_t size, Buffer::Usage usage, VkMemoryRequirements* memoryRequirements);
		void createCommandPool(Engine::Handle engine, VkCommandPool* pool, VkCommandPoolCreateFlags flags);
		void createCommandBuffers(Engine::Handle engine, VkCommandPool pool, VkCommandBuffer* commandBuffers, uint64_t count);

		void beginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usage);
		void endCommandBuffer(VkCommandBuffer* commandBuffers, uint64_t count, VkQueue queue);

		void createOneTimeStagingBuffer(Engine::Handle engine, VkBuffer* buffer, VkDeviceMemory* memory, uint64_t size, void** mapping);
		void freeOneTimeStagingBuffer(Engine::Handle engine, VkBuffer buffer, VkDeviceMemory memory);

		void transitionImageLayout(VkImage image, VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkAccessFlags sourceAccess, VkAccessFlags destinationAccess, VkImageMemoryBarrier* barrier);

		void createImage(Engine::Handle engine, VkImage* image, uint32_t width, uint32_t height, Texture::Format format, VkSampleCountFlagBits sampleCount, VkImageLayout initialLayout, VkImageUsageFlags usage);
		void createView(Engine::Handle engine, VkImageView* view, Texture::Format format, VkImage image);
		void createSampler(Engine::Handle engine, VkSampler* sampler, Texture::Filter filter);

		void moveBufferToImage(VkBuffer imageBuffer, VkImage image, VkCommandBuffer commandBuffer, uint32_t width, uint32_t height, VkBufferImageCopy* region);

		namespace Context {
			struct Resource {
				typedef std::vector<std::function<void(void)>> FunctionArray;

				std::array<FunctionArray, 2> perFrameCleanupArrays;
				uint32_t frameIndex;
				VkCommandBuffer currentCommandBuffer;

				VkCommandPool transferPool;
				VkCommandBuffer transferCommandBuffer;

				bool inTransfer;

				const VkDeviceSize literalZero;

				Resource();
				~Resource();

				bool isNull() const;
				void addFunction(std::function<void(void)> function);

				// Public
				void drop(Engine::Handle engine);
				void create(Engine::Handle engine);

				void beginTransfer();
				void endTransfer(Engine::Handle engine);

				void flush(Engine::Handle engine);
			};

			typedef Resource* Handle;

			template <Allocator::AllocatorType AllocatorType>
			Handle create(AllocatorType allocator, Engine::Handle engine)
			{
				Handle handle = Allocator::allocateResource<Resource>(allocator);

				handle->create(engine);

				return handle;
			}

			void flush(Handle context, Engine::Handle engine);
			void beginTransfer(Handle context);
			void endTransfer(Handle context, Engine::Handle engine);

			template <Allocator::AllocatorType AllocatorType>
			void drop(AllocatorType allocator, Handle handle, Engine::Handle engine)
			{
				handle->drop(engine);

				Allocator::dropResource(allocator, handle);
			}
		}

		void transferBuffer(Context::Handle context, Buffer::Handle source, Buffer::Handle destination);

		void bindPipeline(Context::Handle context, Pipeline::Handle handle);
		void bindVertexBuffer(Context::Handle context, Buffer::Handle handle);
		void bindIndexBuffer(Context::Handle context, Buffer::Handle handle);
		void bindDescriptorSet(Context::Handle context, DescriptorSet::Handle descriptorSet, Pipeline::Handle pipeline);
		void bindDescriptorSetDynamic(Context::Handle context, DescriptorSet::Handle descriptorSet, Pipeline::Handle pipeline, const std::span<const uint32_t>& offsets);

		void pushConstants(Context::Handle context, const void* data, uint64_t size, uint64_t offset, Shader::Stage stage, Pipeline::Handle pipeline);

		void drawIndexed(Context::Handle context, uint32_t indexCount, uint32_t instanceCount);
	}
}