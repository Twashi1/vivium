#include "commands.h"

namespace Vivium {
	namespace Commands {
		namespace Context {
			Resource::Resource()
				: frameIndex(0), literalZero(0), inTransfer(false), transferPool(VK_NULL_HANDLE), transferCommandBuffer(VK_NULL_HANDLE)
			{}

			Resource::~Resource()
			{
				for (FunctionArray cleanupArray : perFrameCleanupArrays) {
					for (std::function<void(void)> function : cleanupArray) {
						function();
					}
				}
			}

			void Resource::addFunction(std::function<void(void)> function)
			{
				perFrameCleanupArrays[frameIndex].push_back(function);
			}

			void Resource::drop(Engine::Handle engine)
			{
				vkFreeCommandBuffers(engine->device, transferPool, 1, &transferCommandBuffer);
				vkDestroyCommandPool(engine->device, transferPool, nullptr);
			}

			void Resource::create(Engine::Handle engine)
			{
				Commands::createCommandPool(engine, &transferPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
				Commands::createCommandBuffers(engine, transferPool, &transferCommandBuffer, 1);
			}
			
			void Resource::beginTransfer()
			{
				Commands::beginCommandBuffer(transferCommandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

				inTransfer = true;
			}
			
			void Resource::endTransfer(Engine::Handle engine)
			{
				Commands::endCommandBuffer(&transferCommandBuffer, 1, engine->transferQueue);
				
				// TODO: bad
				vkQueueWaitIdle(engine->transferQueue);
				
				inTransfer = false;
			}
			
			void Resource::flush(Engine::Handle engine)
			{
				frameIndex = (frameIndex + 1) % 2;
				currentCommandBuffer = engine->commandBuffers[engine->currentFrameIndex];

				for (std::function<void(void)> function : perFrameCleanupArrays[frameIndex]) {
					function();
				}

				perFrameCleanupArrays[frameIndex].clear();
			}

			void flush(Handle context, Engine::Handle engine)
			{
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(context, Context::isNull);

				context->flush(engine);
			}
			
			void beginTransfer(Handle context)
			{
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(context, Context::isNull);

				context->beginTransfer();
			}
			
			void endTransfer(Handle context, Engine::Handle engine)
			{
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(context, Context::isNull);

				context->endTransfer(engine);
			}
			
			bool isNull(const Handle context)
			{
				return context->transferPool == VK_NULL_HANDLE;
			}
		}

		void createBuffer(Engine::Handle engine, VkBuffer* buffer, uint64_t size, Buffer::Usage usage, VkMemoryRequirements* memoryRequirements, const VkAllocationCallbacks* allocationCallbacks) {
			VkBufferCreateInfo bufferCreateInfo{};
			bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCreateInfo.size = size;
			bufferCreateInfo.usage = static_cast<VkBufferUsageFlags>(usage);
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VIVIUM_VK_CHECK(vkCreateBuffer(engine->device, &bufferCreateInfo, allocationCallbacks, buffer), "Failed to create buffer");
			vkGetBufferMemoryRequirements(engine->device, *buffer, memoryRequirements);
		}

		void createCommandPool(Engine::Handle engine, VkCommandPool* pool, VkCommandPoolCreateFlags flags) {
			VkCommandPoolCreateInfo poolCreateInfo{};
			poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolCreateInfo.flags = flags;

			VIVIUM_VK_CHECK(
				vkCreateCommandPool(engine->device, &poolCreateInfo, nullptr, pool),
				"Failed to create command pool"
			);
		}

		void createCommandBuffers(Engine::Handle engine, VkCommandPool pool, VkCommandBuffer* commandBuffers, uint64_t count)
		{
			VkCommandBufferAllocateInfo allocateCreateInfo{};
			allocateCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocateCreateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocateCreateInfo.commandPool = pool;
			allocateCreateInfo.commandBufferCount = static_cast<uint32_t>(count);

			VIVIUM_VK_CHECK(vkAllocateCommandBuffers(
				engine->device,
				&allocateCreateInfo,
				commandBuffers
			), "Failed to allocate command buffers");
		}
		
		void beginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usage)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = usage;

			VIVIUM_VK_CHECK(vkBeginCommandBuffer(
				commandBuffer,
				&beginInfo
			), "Failed to begin command buffer");
		}

		void endCommandBuffer(VkCommandBuffer* commandBuffers, uint64_t count, VkQueue queue)
		{
			for (uint64_t i = 0; i < count; i++) {
				VIVIUM_VK_CHECK(vkEndCommandBuffer(
					commandBuffers[i]
				),
					"Failed to end command buffer"
				);
			}

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = static_cast<uint32_t>(count);
			submitInfo.pCommandBuffers = commandBuffers;

			vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		}

		void createOneTimeStagingBuffer(Engine::Handle engine, VkBuffer* buffer, VkDeviceMemory* memory, uint64_t size, void** mapping)
		{
			VkMemoryRequirements memoryRequirements;
			createBuffer(engine, buffer, size, Buffer::Usage::STAGING, &memoryRequirements, nullptr);

			VkMemoryAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocateInfo.allocationSize = memoryRequirements.size;
			allocateInfo.memoryTypeIndex = findMemoryType(
				engine,
				memoryRequirements.memoryTypeBits,
				static_cast<VkMemoryPropertyFlags>(MemoryType::STAGING)
			);

			VIVIUM_VK_CHECK(vkAllocateMemory(
				engine->device,
				&allocateInfo,
				nullptr,
				memory
			), "Failed to allocate memory");

			VIVIUM_VK_CHECK(vkMapMemory(
				engine->device,
				*memory,
				0,
				size,
				NULL,
				mapping
			), "Failed to map memory");

			VIVIUM_VK_CHECK(vkBindBufferMemory(
				engine->device,
				*buffer,
				*memory,
				0
			), "Failed to bind buffer to memory");
		}

		void freeOneTimeStagingBuffer(Engine::Handle engine, VkBuffer buffer, VkDeviceMemory memory)
		{
			vkDestroyBuffer(engine->device, buffer, nullptr);
			vkUnmapMemory(engine->device, memory);
			vkFreeMemory(engine->device, memory, nullptr);
		}

		void transitionImageLayout(VkImage image, VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkAccessFlags sourceAccess, VkAccessFlags destinationAccess, VkImageMemoryBarrier* barrier)
		{
			barrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier->image = image;
			barrier->subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier->subresourceRange.baseMipLevel = 0;
			barrier->subresourceRange.levelCount = 1;
			barrier->subresourceRange.baseArrayLayer = 0;
			barrier->subresourceRange.layerCount = 1;
			barrier->oldLayout = oldLayout;
			barrier->newLayout = newLayout;
			barrier->srcAccessMask = sourceAccess;
			barrier->dstAccessMask = destinationAccess;

			vkCmdPipelineBarrier(
				commandBuffer,
				sourceStage,
				destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, barrier
			);
		}

		void createImage(Engine::Handle engine, VkImage* image, uint32_t width, uint32_t height, Texture::Format format, VkSampleCountFlagBits sampleCount, VkImageLayout initialLayout, VkImageUsageFlags usage, const VkAllocationCallbacks* allocationCallbacks)
		{
			VkImageCreateInfo imageCreateInfo{};
			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.extent.width = width;
			imageCreateInfo.extent.height = height;
			imageCreateInfo.extent.depth = 1;
			imageCreateInfo.mipLevels = 1;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.format = static_cast<VkFormat>(format);
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo.initialLayout = initialLayout;
			imageCreateInfo.usage = usage;
			imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageCreateInfo.samples = sampleCount;
			imageCreateInfo.flags = 0;

			VIVIUM_VK_CHECK(vkCreateImage(
				engine->device,
				&imageCreateInfo,
				allocationCallbacks,
				image),
				"Failed to create image"
			);
		}

		void createView(Engine::Handle engine, VkImageView* view, Texture::Format format, VkImage image, const VkAllocationCallbacks* allocationCallbacks)
		{
			VkImageViewCreateInfo viewCreateInfo{};
			viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCreateInfo.image = image;
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewCreateInfo.format = static_cast<VkFormat>(format);
			viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewCreateInfo.subresourceRange.baseMipLevel = 0;
			viewCreateInfo.subresourceRange.levelCount = 1;
			viewCreateInfo.subresourceRange.baseArrayLayer = 0;
			viewCreateInfo.subresourceRange.layerCount = 1;

			VIVIUM_VK_CHECK(vkCreateImageView(
				engine->device,
				&viewCreateInfo,
				allocationCallbacks,
				view),
				"Failed to create image view"
			);
		}

		void createSampler(Engine::Handle engine, VkSampler* sampler, Texture::Filter filter, const VkAllocationCallbacks* allocationCallbacks)
		{
			VkSamplerCreateInfo samplerCreateInfo{};
			samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			// TODO: allow customisation of filters
			samplerCreateInfo.magFilter = static_cast<VkFilter>(filter);
			samplerCreateInfo.minFilter = static_cast<VkFilter>(filter);
			samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			// TODO: allow enable
			samplerCreateInfo.anisotropyEnable = VK_FALSE;
			// TODO: allow settting of max anisotropy
			samplerCreateInfo.maxAnisotropy = 1.0f;
			samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
			samplerCreateInfo.compareEnable = VK_FALSE;
			samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerCreateInfo.mipLodBias = 0.0f;
			samplerCreateInfo.minLod = 0.0f;
			samplerCreateInfo.maxLod = 0.0f;

			VIVIUM_VK_CHECK(vkCreateSampler(
				engine->device,
				&samplerCreateInfo,
				allocationCallbacks,
				sampler
			), "Failed to create texture sampler");
		}

		void moveBufferToImage(VkBuffer imageBuffer, VkImage image, VkCommandBuffer commandBuffer, uint32_t width, uint32_t height, VkBufferImageCopy* region)
		{
			region->bufferOffset = 0;
			region->bufferRowLength = 0;
			region->bufferImageHeight = 0;
			region->imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region->imageSubresource.mipLevel = 0;
			region->imageSubresource.baseArrayLayer = 0;
			region->imageSubresource.layerCount = 1;
			region->imageOffset = { 0, 0, 0 };
			region->imageExtent = {
				width,
				height,
				1
			};

			vkCmdCopyBufferToImage(
				commandBuffer,
				imageBuffer,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				region
			);
		}

		void transferBuffer(Context::Handle context, Buffer::Handle source, Buffer::Handle destination) {
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(source, Buffer::isNull);
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(destination, Buffer::isNull);
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(context, Context::isNull);

			VkBufferCopy* copyRegion = new VkBufferCopy;
			copyRegion->srcOffset = 0;
			copyRegion->dstOffset = 0;
			copyRegion->size = source->size;

			vkCmdCopyBuffer(
				context->inTransfer ? context->transferCommandBuffer : context->currentCommandBuffer,
				source->buffer,
				destination->buffer,
				1,
				copyRegion
			);

			context->addFunction([copyRegion]() { delete copyRegion; });
		}

		void bindPipeline(Context::Handle context, Pipeline::Handle handle) {
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(handle, Pipeline::isNull);
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(context, Context::isNull);

			vkCmdBindPipeline(context->currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, handle->pipeline);
		}

		void bindVertexBuffer(Context::Handle context, Buffer::Handle handle)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(handle, Buffer::isNull);
			VIVIUM_ASSERT(handle->usage == Buffer::Usage::VERTEX,
				"Buffer bound was not a vertex buffer");

			vkCmdBindVertexBuffers(
				context->currentCommandBuffer,
				0,
				1,
				&(handle->buffer),
				&context->literalZero
			);
		}

		void bindIndexBuffer(Context::Handle context, Buffer::Handle handle)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(handle, Buffer::isNull);
			VIVIUM_ASSERT(handle->usage == Buffer::Usage::INDEX,
				"Buffer bound was not an index buffer");

			vkCmdBindIndexBuffer(
				context->currentCommandBuffer,
				handle->buffer,
				0,
				VK_INDEX_TYPE_UINT16
			);
		}

		void bindDescriptorSet(Context::Handle context, DescriptorSet::Handle descriptorSet, Pipeline::Handle pipeline)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(descriptorSet, DescriptorSet::isNull);
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(pipeline, Pipeline::isNull);

			vkCmdBindDescriptorSets(
				context->currentCommandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipeline->layout,
				0,
				1,
				&(descriptorSet->descriptorSet),
				0,
				nullptr
			);
		}

		void bindDescriptorSetDynamic(Context::Handle context, DescriptorSet::Handle descriptorSet, Pipeline::Handle pipeline, const std::span<const uint32_t>& offsets)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(descriptorSet, DescriptorSet::isNull);
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(pipeline, Pipeline::isNull);

			vkCmdBindDescriptorSets(
				context->currentCommandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipeline->layout,
				0,
				1,
				&(descriptorSet->descriptorSet),
				static_cast<uint32_t>(offsets.size()),
				offsets.data()
			);
		}

		void pushConstants(Context::Handle context, const void* data, uint64_t size, uint64_t offset, Shader::Stage stage, Pipeline::Handle pipeline)
		{
			vkCmdPushConstants(
				context->inTransfer ? context->transferCommandBuffer : context->currentCommandBuffer,
				pipeline->layout,
				static_cast<VkShaderStageFlags>(stage),
				static_cast<uint32_t>(offset),
				static_cast<uint32_t>(size),
				data
			);
		}

		void drawIndexed(Context::Handle context, uint32_t indexCount, uint32_t instanceCount)
		{
			vkCmdDrawIndexed(
				context->currentCommandBuffer,
				indexCount,
				instanceCount,
				0,
				0,
				0
			);
		}
	}
}