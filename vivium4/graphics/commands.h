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
	struct ResourceManager;

	uint32_t _findMemoryType(Engine& engine, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	// TODO: these still take allocation callbacks
	void _cmdCreateBuffer(Engine& engine, VkBuffer* buffer, uint64_t size, BufferUsage usage, VkMemoryRequirements* memoryRequirements, const VkAllocationCallbacks* allocationCallbacks);
	void _cmdCreateCommandPool(Engine& engine, VkCommandPool* pool, VkCommandPoolCreateFlags flags);
	void _cmdCreateCommandBuffers(Engine& engine, VkCommandPool pool, VkCommandBuffer* commandBuffers, uint64_t count);

	void _cmdBeginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usage);
	void _cmdEndCommandBuffer(VkCommandBuffer* commandBuffers, uint64_t count, VkQueue queue);

	void _cmdCreateTransientStagingBuffer(Engine& engine, VkBuffer* buffer, VkDeviceMemory* memory, uint64_t size, void** mapping);
	void _cmdFreeTransientStagingBuffer(Engine& engine, VkBuffer buffer, VkDeviceMemory memory);

	void _cmdTransitionImageLayout(VkImage image, VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkAccessFlags sourceAccess, VkAccessFlags destinationAccess, VkImageMemoryBarrier* barrier);

	void _cmdCreateImage(Engine& engine, VkImage* image, uint32_t width, uint32_t height, TextureFormat format, VkSampleCountFlagBits sampleCount, VkImageLayout initialLayout, VkImageUsageFlags usage, const VkAllocationCallbacks* allocationCallbacks);
	void _cmdCreateView(Engine& engine, VkImageView* view, TextureFormat format, VkImage image, const VkAllocationCallbacks* allocationCallbacks);
	void _cmdCreateSampler(Engine& engine, VkSampler* sampler, TextureFilter filter, const VkAllocationCallbacks* allocationCallbacks);
		
	void _cmdCreatePipeline(Engine& engine, ResourceManager& manager, VkPipeline* pipeline, VkPipelineLayout* layout, VkRenderPass renderPass, const std::span<const ShaderReference> shaders, const std::span<const DescriptorLayoutReference> descriptorLayouts, const std::span<const PushConstant> pushConstants, BufferLayout const& bufferLayout, VkSampleCountFlagBits sampleCount, const VkAllocationCallbacks* layoutAllocationCallback, const VkAllocationCallbacks* pipelineAllocationCallback);
	// TODO: need more options in order to use this to create a framebuffer render pass
	void _cmdCreateRenderPass(Engine& engine, VkRenderPass* renderPass, VkFormat imageFormat, VkSampleCountFlagBits sampleCount);

	void _cmdCopyBufferToImage(VkBuffer imageBuffer, VkImage image, VkCommandBuffer commandBuffer, uint32_t width, uint32_t height, VkBufferImageCopy* region);
	
	// TODO: some global for this? maybe vulkan has a good default for it as well?
	inline const VkDeviceSize _literalZero = 0;

	struct CommandContext {
		// TODO: yuck... look towards some proper temporary memory
		typedef std::vector<std::function<void(void)>> FunctionArray;

		std::array<FunctionArray, 2> perFrameCleanupArrays;
		uint32_t frameIndex;
		VkCommandBuffer currentCommandBuffer;

		VkCommandPool transferPool;
		VkCommandBuffer transferCommandBuffer;

		bool inTransfer;
	};

	CommandContext createCommandContext(Engine& engine);

	void _contextAddFunction(CommandContext& context, std::function<void(void)> function);
	// TODO: must be done by user, not private
	void _contextFlush(CommandContext& context, Engine& engine);
	void contextBeginTransfer(CommandContext& context);
	void contextEndTransfer(CommandContext& context, Engine& engine);

	void dropCommandContext(CommandContext& context, Engine& engine);

	// TODO: allow passing region/buffer slice
	void cmdTransferBuffer(CommandContext& context, Buffer const& source, uint64_t sourceSize, uint64_t sourceOffset, Buffer& destination);

	void cmdBindPipeline(CommandContext& context, Pipeline const& handle);
	void cmdBindVertexBuffer(CommandContext& context, Buffer const& handle);
	void cmdBindIndexBuffer(CommandContext& context, Buffer const& handle);
	void cmdBindDescriptorSet(CommandContext& context, DescriptorSet const& descriptorSet, Pipeline const& pipeline);

	void cmdWritePushConstants(CommandContext& context, const void* data, uint64_t size, uint64_t offset, ShaderStage stage, Pipeline const& pipeline);

	void cmdDrawIndexed(CommandContext& context, uint32_t indexCount, uint32_t instanceCount);
}