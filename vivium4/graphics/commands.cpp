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

		void createRenderPass(Engine::Handle engine, VkRenderPass* renderPass, VkFormat imageFormat, VkSampleCountFlagBits sampleCount) {
			// TODO: lots of code required for making this work when not multisampling
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = imageFormat;
			colorAttachment.samples = sampleCount;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = sampleCount & VK_SAMPLE_COUNT_1_BIT ?
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription colorAttachmentResolve{};
			colorAttachmentResolve.format = imageFormat;
			colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference colorAttachmentResolveRef{};
			colorAttachmentResolveRef.attachment = 1;
			colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			if (!(sampleCount & VK_SAMPLE_COUNT_1_BIT)) {
				subpass.pResolveAttachments = &colorAttachmentResolveRef;
			}

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = VK_ACCESS_NONE;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkAttachmentDescription attachments[2] = {
				colorAttachment,
				colorAttachmentResolve // Only include in non-multisampled image
			};

			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = sampleCount & VK_SAMPLE_COUNT_1_BIT ? 1 : 2;
			renderPassInfo.pAttachments = attachments;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			VIVIUM_VK_CHECK(vkCreateRenderPass(engine->device, &renderPassInfo, nullptr, renderPass),
				"Failed to create render pass");
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

		void createPipeline(Engine::Handle engine, VkPipeline* pipeline, VkPipelineLayout* layout, VkRenderPass renderPass, const std::span<const Shader::Handle> shaders, const std::span<const DescriptorLayout::Handle> descriptorLayouts, const std::span<const Uniform::PushConstant> pushConstants, const Buffer::Layout& bufferLayout, VkSampleCountFlagBits sampleCount, const VkAllocationCallbacks* layoutAllocationCallback, const VkAllocationCallbacks* pipelineAllocationCallback) {
			std::vector<VkPipelineShaderStageCreateInfo> shaderStages(shaders.size());

			for (uint32_t i = 0; i < shaders.size(); i++) {
				Shader::Handle shader = shaders[i];

				VkPipelineShaderStageCreateInfo shaderStageInfo{};
				shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shaderStageInfo.stage = shader->flags;
				shaderStageInfo.module = shader->shader;
				shaderStageInfo.pName = "main";

				shaderStages[i] = shaderStageInfo;
			}

			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(bufferLayout.attributeDescriptions.size());
			vertexInputInfo.pVertexBindingDescriptions = &(bufferLayout.bindingDescription);
			vertexInputInfo.pVertexAttributeDescriptions = bufferLayout.attributeDescriptions.data();

			VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			VkPipelineViewportStateCreateInfo viewportState{};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.scissorCount = 1;

			VkPipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rasterizer.depthBiasEnable = VK_FALSE;

			VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = sampleCount;

			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

			// TODO: possibly re-enable, tested transparency?
			VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			// colorBlending.logicOpEnable = VK_FALSE;
			// colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
			// colorBlending.blendConstants[0] = 0.0f;
			// colorBlending.blendConstants[1] = 0.0f;
			// colorBlending.blendConstants[2] = 0.0f;
			// colorBlending.blendConstants[3] = 0.0f;

			std::vector<VkDynamicState> dynamicStates = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};

			VkPipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
			dynamicState.pDynamicStates = dynamicStates.data();

			std::vector<VkDescriptorSetLayout> vulkanDescriptorLayouts(descriptorLayouts.size());

			for (uint32_t i = 0; i < descriptorLayouts.size(); i++) {
				vulkanDescriptorLayouts[i] = descriptorLayouts[i]->layout;
			}

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.pSetLayouts = vulkanDescriptorLayouts.data();
			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(vulkanDescriptorLayouts.size());
			// TODO: dirty, better to just make the copy
			pipelineLayoutInfo.pPushConstantRanges = reinterpret_cast<const VkPushConstantRange*>(pushConstants.data());
			pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());

			VIVIUM_VK_CHECK(vkCreatePipelineLayout(
				engine->device,
				&pipelineLayoutInfo,
				layoutAllocationCallback,
				layout),
				"Failed to create pipeline layout"
			);

			VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
			pipelineInfo.pStages = shaderStages.data();
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = nullptr;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.pDynamicState = &dynamicState;
			pipelineInfo.layout = *layout;
			pipelineInfo.renderPass = renderPass;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			pipelineInfo.basePipelineIndex = -1;

			VIVIUM_VK_CHECK(vkCreateGraphicsPipelines(
				engine->device,
				VK_NULL_HANDLE,
				1,
				&pipelineInfo,
				pipelineAllocationCallback,
				pipeline
			), "Failed to create graphics pipeline");
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