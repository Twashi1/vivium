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
		// TODO: a lot of these should be private
		void createBuffer(Engine::Handle engine, VkBuffer* buffer, uint64_t size, BufferUsage usage, VkMemoryRequirements* memoryRequirements, const VkAllocationCallbacks* allocationCallbacks);
		void createCommandPool(Engine::Handle engine, VkCommandPool* pool, VkCommandPoolCreateFlags flags);
		void createCommandBuffers(Engine::Handle engine, VkCommandPool pool, VkCommandBuffer* commandBuffers, uint64_t count);

		void beginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usage);
		void endCommandBuffer(VkCommandBuffer* commandBuffers, uint64_t count, VkQueue queue);

		void createOneTimeStagingBuffer(Engine::Handle engine, VkBuffer* buffer, VkDeviceMemory* memory, uint64_t size, void** mapping);
		void freeOneTimeStagingBuffer(Engine::Handle engine, VkBuffer buffer, VkDeviceMemory memory);

		void transitionImageLayout(VkImage image, VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkAccessFlags sourceAccess, VkAccessFlags destinationAccess, VkImageMemoryBarrier* barrier);

		void createImage(Engine::Handle engine, VkImage* image, uint32_t width, uint32_t height, TextureFormat format, VkSampleCountFlagBits sampleCount, VkImageLayout initialLayout, VkImageUsageFlags usage, const VkAllocationCallbacks* allocationCallbacks);
		void createView(Engine::Handle engine, VkImageView* view, TextureFormat format, VkImage image, const VkAllocationCallbacks* allocationCallbacks);
		void createSampler(Engine::Handle engine, VkSampler* sampler, TextureFilter filter, const VkAllocationCallbacks* allocationCallbacks);
		
		void createPipeline(Engine::Handle engine, VkPipeline* pipeline, VkPipelineLayout* layout, VkRenderPass renderPass, const std::span<const ShaderReference> shaders, const std::span<const DescriptorLayoutReference> descriptorLayouts, const std::span<const PushConstant> pushConstants, BufferLayout const& bufferLayout, VkSampleCountFlagBits sampleCount, const VkAllocationCallbacks* layoutAllocationCallback, const VkAllocationCallbacks* pipelineAllocationCallback);
		// TODO: need more options in order to use this to create a framebuffer render pass
		void createRenderPass(Engine::Handle engine, VkRenderPass* renderPass, VkFormat imageFormat, VkSampleCountFlagBits sampleCount);

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

				void addFunction(std::function<void(void)> function);

				// Public
				void drop(Engine::Handle engine);
				void create(Engine::Handle engine);

				void beginTransfer();
				void endTransfer(Engine::Handle engine);

				void flush(Engine::Handle engine);
			};

			typedef Resource* Handle;

			template <Storage::StorageType StorageType>
			Handle create(StorageType* allocator, Engine::Handle engine)
			{
				Handle handle = Storage::allocateResource<Resource>(allocator);

				handle->create(engine);

				return handle;
			}

			void flush(Handle context, Engine::Handle engine);
			void beginTransfer(Handle context);
			void endTransfer(Handle context, Engine::Handle engine);
			bool isNull(const Handle context);

			template <Storage::StorageType StorageType>
			void drop(StorageType* allocator, Handle handle, Engine::Handle engine)
			{
				handle->drop(engine);

				Storage::dropResource(allocator, handle);
			}
		}

		// TODO: allow passing region/buffer slice
		void transferBuffer(Context::Handle context, Buffer const& source, uint64_t sourceSize, Buffer& destination);

		void bindPipeline(Context::Handle context, Pipeline const& handle);
		void bindVertexBuffer(Context::Handle context, Buffer const& handle);
		void bindIndexBuffer(Context::Handle context, Buffer const& handle);
		void bindDescriptorSet(Context::Handle context, DescriptorSet const& descriptorSet, Pipeline const& pipeline);

		void pushConstants(Context::Handle context, const void* data, uint64_t size, uint64_t offset, ShaderStage stage, Pipeline const& pipeline);

		void drawIndexed(Context::Handle context, uint32_t indexCount, uint32_t instanceCount);
	}
}