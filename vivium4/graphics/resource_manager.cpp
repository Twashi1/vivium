#include "resource_manager.h"

namespace Vivium {
	ResourceManager::DeviceMemoryHandle::DeviceMemoryHandle()
		: memory(VK_NULL_HANDLE), mapping(nullptr) {}

	ResourceManager::DeviceMemoryHandle _allocateDeviceMemory(ResourceManager& manager, Engine& engine, uint32_t memoryTypeBits, MemoryType memoryType, uint64_t size) {
		ResourceManager::DeviceMemoryHandle deviceMemoryHandle;

		VkMemoryAllocateInfo memoryAllocationInfo{};
		memoryAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocationInfo.allocationSize = size;
		memoryAllocationInfo.memoryTypeIndex = findMemoryType(
			engine,
			memoryTypeBits,
			static_cast<VkMemoryPropertyFlags>(memoryType)
		);

		VIVIUM_VK_CHECK(vkAllocateMemory(
			engine.device,
			&memoryAllocationInfo,
			nullptr,
			&deviceMemoryHandle.memory
		), "Failed to allocate memory of size {} and type {}",
			size,
			memoryAllocationInfo.memoryTypeIndex
		);

		if (isMappable(memoryType)) {
			VIVIUM_VK_CHECK(
				vkMapMemory(
					engine.device,
					deviceMemoryHandle.memory,
					0,
					size,
					NULL,
					&deviceMemoryHandle.mapping
				),
				"Failed to map memory"
			);
		}

		manager.deviceMemoryHandles.push_back(deviceMemoryHandle);

		return deviceMemoryHandle;
	}

	void _allocateBuffers(ResourceManager& manager, Engine& engine, MemoryType memoryType)
	{
		std::vector<BufferSpecification>* bufferSpecificationsPtr;
		std::vector<Buffer>* bufferMemoryPtr;

		// TODO: should be checking for a specific tag (DEVICE_LOCAL)
		switch (memoryType) {
		case MemoryType::STAGING:
			bufferSpecificationsPtr = &manager.hostBuffers.specifications;
			bufferMemoryPtr = &manager.hostBuffers.resources;
			break;
		case MemoryType::DEVICE:
			bufferSpecificationsPtr = &manager.deviceBuffers.specifications;
			bufferMemoryPtr = &manager.deviceBuffers.resources;
			break;
		default: VIVIUM_LOG(LogSeverity::FATAL, "Invalid memory type to allocate buffer to"); break;
		}

		std::vector<BufferSpecification>& bufferSpecifications = *bufferSpecificationsPtr;
		std::vector<Buffer>& bufferMemory = *bufferMemoryPtr;
				
		uint64_t totalSize = 0;

		std::vector<uint64_t> bufferOffsets(bufferSpecifications.size());
		uint32_t memoryTypeBits = 0;

		for (uint64_t i = 0; i < bufferSpecifications.size(); i++) {
			Buffer& resource = bufferMemory[i];
			BufferSpecification const& specification = bufferSpecifications[i];

			VkMemoryRequirements memoryRequirements;

			// Create the VkBuffer and get the memory requirements
			_cmdCreateBuffer(engine, &resource.buffer, specification.size, specification.usage, &memoryRequirements, nullptr);
			// Calculate offset this buffer should be at in the device memory
			uint64_t resourceOffset = nearestMultiple(totalSize, memoryRequirements.alignment);
			bufferOffsets[i] = resourceOffset;
			totalSize = resourceOffset + memoryRequirements.size;

			// Include memory type bits
			memoryTypeBits |= memoryRequirements.memoryTypeBits;
		}

		// Get some device memory for these buffers
		ResourceManager::DeviceMemoryHandle deviceMemoryHandle = _allocateDeviceMemory(
			manager,
			engine,
			memoryTypeBits,
			memoryType,
			totalSize
		);

		// Bind buffers to memory
		for (uint64_t i = 0; i < bufferSpecifications.size(); i++) {
			Buffer& resource = bufferMemory[i];

			vkBindBufferMemory(engine.device, resource.buffer, deviceMemoryHandle.memory, bufferOffsets[i]);

			if (deviceMemoryHandle.mapping != nullptr) {
				resource.mapping = reinterpret_cast<uint8_t*>(deviceMemoryHandle.mapping) + bufferOffsets[i];
			}
		}
	}

	void _allocateShaders(ResourceManager& manager, Engine& engine)
	{
		for (uint64_t i = 0; i < manager.shaders.specifications.size(); i++) {
			ShaderSpecification const& specification = manager.shaders.specifications[i];
			Shader& resource = manager.shaders.resources[i];
					
			VkShaderModuleCreateInfo shaderCreateInfo{};
			shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shaderCreateInfo.codeSize = specification.length;
			shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(specification.code.c_str());

			VIVIUM_VK_CHECK(vkCreateShaderModule(engine.device, &shaderCreateInfo, nullptr, &resource.shader),
				"Failed to create shader module");
		}
	}

	void _allocateDescriptorLayouts(ResourceManager& manager, Engine& engine)
	{
		for (uint64_t i = 0; i < manager.descriptorLayouts.specifications.size(); i++) {
			DescriptorLayoutSpecification const& specification = manager.descriptorLayouts.specifications[i];
			DescriptorLayout& resource = manager.descriptorLayouts.resources[i];

			std::vector<VkDescriptorSetLayoutBinding> vulkanBindings(specification.bindings.size());

			for (uint64_t i = 0; i < specification.bindings.size(); i++) {
				VkDescriptorSetLayoutBinding& vulkanBinding = vulkanBindings[i];
				const UniformBinding& binding = specification.bindings[i];

				vulkanBinding.binding = binding.slot;
				vulkanBinding.descriptorCount = 1; // TODO: UBO arrays would require this to be editable (would it?)
				vulkanBinding.descriptorType = static_cast<VkDescriptorType>(binding.type);
				vulkanBinding.stageFlags = static_cast<VkShaderStageFlags>(binding.stage);
				vulkanBinding.pImmutableSamplers = nullptr;
			}

			VkDescriptorSetLayoutCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			createInfo.bindingCount = static_cast<uint32_t>(vulkanBindings.size());
			createInfo.pBindings = vulkanBindings.data();

			VIVIUM_VK_CHECK(vkCreateDescriptorSetLayout(engine.device, &createInfo, nullptr, &resource.layout),
				"Failed to create descriptor set layout");
		}
	}

	void _allocateTextures(ResourceManager& manager, Engine& engine)
	{
		// TODO: assert number of specifications passed is less than the number
		// of device memory blocks we can allocate,
		// otherwise we need to batch these staging buffers together (which we should do anyway)

		// For first transition, buffer copy, second transition
		std::array<VkCommandBuffer, 3> commandBuffers;
		VkCommandPool commandPool;

		// Create command pool
		_cmdCreateCommandPool(engine, &commandPool, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
		// Create command buffers
		_cmdCreateCommandBuffers(engine, commandPool, commandBuffers.data(), commandBuffers.size());

		// For storing temporary buffers, buffer image copy, pipeline barriers
		std::vector<VkBuffer> textureBuffers;
		std::vector<VkDeviceMemory> textureBufferMemories;
		std::vector<VkImageMemoryBarrier> textureBarriers;
		std::vector<VkBufferImageCopy> textureRegions;

		// Reserve space for texture temporaries
		uint64_t specificationCount = manager.textures.specifications.size();
		textureBarriers.reserve(specificationCount * 2);
		textureBuffers.reserve(specificationCount);
		textureRegions.reserve(specificationCount);
		textureBufferMemories.reserve(specificationCount);

		// Calculate total size required
		uint64_t totalSize = 0;
		uint32_t memoryTypeBits = 0;
		std::vector<uint64_t> offsets(specificationCount);

		for (uint64_t i = 0; i < manager.textures.specifications.size(); i++) {
			Texture& texture = manager.textures.resources[i];
			TextureSpecification const& specification = manager.textures.specifications[i];

			_cmdCreateImage(
				engine,
				&texture.image,
				specification.width,
				specification.height,
				specification.imageFormat,
				// TODO: through some method, convert to n samples
				// TODO: cant do it here though, since this image must be a transfer destination
				// maybe not necessary ^^?
				VK_SAMPLE_COUNT_1_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				nullptr
			);

			VkMemoryRequirements memoryRequirements;

			vkGetImageMemoryRequirements(
				engine.device,
				texture.image,
				&memoryRequirements
			);

			uint64_t resourceOffset = nearestMultiple(totalSize, memoryRequirements.alignment);
			offsets[i] = resourceOffset;
			totalSize = resourceOffset + memoryRequirements.size;

			memoryTypeBits |= memoryRequirements.memoryTypeBits;
		}

		// Begin command buffers
		for (uint64_t i = 0; i < commandBuffers.size(); i++) {
			_cmdBeginCommandBuffer(
				commandBuffers[i],
				VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
			);
		}

		// Create VkDeviceMemory
		ResourceManager::DeviceMemoryHandle deviceMemoryHandle = _allocateDeviceMemory(manager, engine, memoryTypeBits, MemoryType::DEVICE, totalSize);

		std::vector<VkDeviceMemory> oneTimeStagingMemories;
		std::vector<VkBuffer> oneTimeStagingBuffers;

		for (uint64_t i = 0; i < manager.textures.specifications.size(); i++) {
			Texture& texture = manager.textures.resources[i];
			TextureSpecification const& specification = manager.textures.specifications[i];
					
			// Bind image to memory
			VIVIUM_VK_CHECK(vkBindImageMemory(
				engine.device,
				texture.image,
				deviceMemoryHandle.memory,
				offsets[i]
			), "Failed to bind image to memory");

			textureBarriers.push_back({});

			_cmdTransitionImageLayout(
				texture.image,
				commandBuffers[0],
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_ACCESS_NONE,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				&textureBarriers.back()
			);

			VkDeviceMemory stagingMemory;
			VkBuffer stagingBuffer;
			void* stagingMapping;

			_cmdCreateTransientStagingBuffer(
				engine,
				&stagingBuffer,
				&stagingMemory,
				specification.data.size(),
				&stagingMapping
			);

			oneTimeStagingBuffers.push_back(stagingBuffer);
			oneTimeStagingMemories.push_back(stagingMemory);

			std::memcpy(stagingMapping, specification.data.data(), specification.data.size());

			textureRegions.push_back({});

			_cmdCopyBufferToImage(
				stagingBuffer,
				texture.image,
				commandBuffers[1],
				specification.width,
				specification.height,
				&textureRegions.back()
			);

			textureBarriers.push_back({});

			_cmdTransitionImageLayout(
				texture.image,
				commandBuffers[2],
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT,
				&textureBarriers.back()
			);
		}

		// TODO: put on transfer queue once confirmed working
		_cmdEndCommandBuffer(
			commandBuffers.data(),
			commandBuffers.size(),
			engine.graphicsQueue
		);
		// TODO: experiment with putting this wait until after creating view and sampler?
		vkQueueWaitIdle(engine.graphicsQueue);

		// TODO: maybe these don't need the image to be completely filled out beforehand?
		for (uint64_t i = 0; i < manager.textures.specifications.size(); i++) {
			Texture& texture = manager.textures.resources[i];
			TextureSpecification const& specification = manager.textures.specifications[i];

			_cmdCreateView(engine, &texture.view, specification.imageFormat, texture.image, nullptr);
			_cmdCreateSampler(engine, &texture.sampler, specification.imageFilter, nullptr);
		}

		// Wait until idle since we will free unnecessary resources now
		vkDeviceWaitIdle(engine.device);

		for (uint32_t i = 0; i < oneTimeStagingBuffers.size(); i++) {
			vkDestroyBuffer(engine.device, oneTimeStagingBuffers[i], nullptr);
			vkUnmapMemory(engine.device, oneTimeStagingMemories[i]);
			vkFreeMemory(engine.device, oneTimeStagingMemories[i], nullptr);
		}

		for (uint64_t i = 0; i < textureBuffers.size(); i++) {
			_cmdFreeTransientStagingBuffer(
				engine,
				textureBuffers[i],
				textureBufferMemories[i]
			);
		}

		vkFreeCommandBuffers(
			engine.device,
			commandPool,
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data()
		);

		vkDestroyCommandPool(engine.device, commandPool, nullptr);
	}

	void _allocateFramebuffers(ResourceManager& manager, Engine& engine)
	{
		uint64_t totalMemoryRequired = 0;
		uint32_t memoryTypeBits = NULL;

		std::vector<uint64_t> imageMemoryLocations(manager.framebuffers.specifications.size());

		// Create images and count memory requirements
		for (uint64_t i = 0; i < manager.framebuffers.specifications.size(); i++) {
			FramebufferSpecification const& specification = manager.framebuffers.specifications[i];
			Framebuffer& resource = manager.framebuffers.resources[i];

			_cmdCreateImage(
				engine,
				&resource.image,
				static_cast<uint32_t>(resource.dimensions.x), static_cast<uint32_t>(resource.dimensions.y),
				specification.format,
				VK_SAMPLE_COUNT_1_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				nullptr
			);

			VkMemoryRequirements requirements;
			vkGetImageMemoryRequirements(engine.device, resource.image, &requirements);

			uint64_t resourceOffset = nearestMultiple(totalMemoryRequired, requirements.alignment);
			imageMemoryLocations[i] = resourceOffset;
			totalMemoryRequired = resourceOffset + requirements.size;

			memoryTypeBits |= requirements.memoryTypeBits;
		}

		ResourceManager::DeviceMemoryHandle memory = _allocateDeviceMemory(manager, engine, memoryTypeBits, MemoryType::DEVICE, totalMemoryRequired);

		for (uint64_t i = 0; i < manager.framebuffers.specifications.size(); i++) {
			FramebufferSpecification const& specification = manager.framebuffers.specifications[i];
			Framebuffer& resource = manager.framebuffers.resources[i];
			
			resource.currentFrameIndex = 0;

			VIVIUM_VK_CHECK(vkBindImageMemory(
				engine.device,
				resource.image,
				memory.memory,
				imageMemoryLocations[i]
			), "Failed to bind image memory");

			_cmdCreateView(engine, &resource.view, specification.format, resource.image, nullptr);
			// TODO: customiseable filter
			_cmdCreateSampler(engine, &resource.sampler, TextureFilter::LINEAR, nullptr);

			// Creating render pass
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = static_cast<VkFormat>(specification.format);
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

			VkSubpassDescription subpassDescription{};
			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = &colorReference;

			// TODO: is setting up a subpass the optimal way to do it?
			//	maybe its better to perform some transitions one-time (if even possible)?
			std::array<VkSubpassDependency, 2> dependencies;
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT |
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_NONE_KHR;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpassDescription;
			renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
			renderPassInfo.pDependencies = dependencies.data();

			VIVIUM_VK_CHECK(vkCreateRenderPass(engine.device, &renderPassInfo, nullptr, &resource.renderPass), "Failed render pass");

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = resource.renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &resource.view;
			framebufferInfo.width = static_cast<uint32_t>(resource.dimensions.x);
			framebufferInfo.height = static_cast<uint32_t>(resource.dimensions.y);
			framebufferInfo.layers = 1;

			VIVIUM_VK_CHECK(vkCreateFramebuffer(engine.device, &framebufferInfo, nullptr, &resource.framebuffer), "Failed to create framebuffer");

			// Create sync objects, command pool, and command buffers
			Engine::QueueFamilyIndices queueFamilyIndices = _findQueueFamilies(engine.physicalDevice);

			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

			VIVIUM_VK_CHECK(vkCreateCommandPool(engine.device, &poolInfo, nullptr, &resource.commandPool),
				"Failed to create command pool");

			VkCommandBufferAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocateInfo.commandPool = resource.commandPool;
			allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocateInfo.commandBufferCount = static_cast<uint32_t>(resource.commandBuffers.size());

			VIVIUM_VK_CHECK(vkAllocateCommandBuffers(engine.device, &allocateInfo, resource.commandBuffers.data()),
				"Failed to allocate command buffers");

			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (uint32_t i = 0; i < VIVIUM_FRAMES_IN_FLIGHT; i++) {
				// TODO: VkCheck this
				if (vkCreateSemaphore(engine.device, &semaphoreInfo, nullptr, &resource.imageAvailableSemaphores[i]) != VK_SUCCESS ||
					vkCreateSemaphore(engine.device, &semaphoreInfo, nullptr, &resource.renderFinishedSemaphores[i]) != VK_SUCCESS ||
					vkCreateFence(engine.device, &fenceInfo, nullptr, &resource.inFlightFences[i]) != VK_SUCCESS) {
					VIVIUM_LOG(LogSeverity::FATAL, "Failed to create sync objects for a frame");
				}
			}
		}
	}

	void _allocateDescriptorSets(ResourceManager& manager, Engine& engine)
	{
		// Create descriptor pool
		// Count descriptor pools
		std::array<VkDescriptorPoolSize, 4> poolSizeCounts = {
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0 },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0 },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0 },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0 }
		};

		{
			// TODO: might be faster now that we know everything is a uint64_t
			// NOTE: this is slow O(n^2), could use std::unordered_set, but doubt it would be faster
			std::vector<DescriptorLayoutReference> seenLayouts;

			for (uint64_t i = 0; i < manager.descriptorSets.specifications.size(); i++) {
				DescriptorSetSpecification const& specification = manager.descriptorSets.specifications[i];

				uint64_t j = 0;
				for (j = 0; j < seenLayouts.size(); j++) {
					DescriptorLayoutReference const& seenLayout = seenLayouts[j];

					if (seenLayout.referenceIndex == specification.layout.referenceIndex) {
						j = UINT64_MAX;

						break;
					}
				}

				if (j == UINT64_MAX) {
					continue;
				}
				else {
					seenLayouts.push_back(specification.layout);
				}

				std::vector<UniformBinding> const& bindings = manager.descriptorLayouts.specifications[specification.layout.referenceIndex].bindings;

				// Required for this to work
				for (UniformBinding const& binding : bindings) {
					switch (binding.type) {
					case UniformType::UNIFORM_BUFFER:
						poolSizeCounts[0].descriptorCount++; break;
					case UniformType::TEXTURE:
					case UniformType::FRAMEBUFFER:
						poolSizeCounts[1].descriptorCount++; break;
					case UniformType::STORAGE_BUFFER:
						poolSizeCounts[2].descriptorCount++; break;
					case UniformType::DYNAMIC_UNIFORM_BUFFER:
						poolSizeCounts[3].descriptorCount++; break;
					default:
						VIVIUM_LOG(LogSeverity::FATAL, "Invalid uniform type"); break;
					}
				}
			}
		}

		std::vector<VkDescriptorPoolSize> nonZeroPoolSizes;
		nonZeroPoolSizes.reserve(4);

		for (VkDescriptorPoolSize poolSize : poolSizeCounts) {
			if (poolSize.descriptorCount != 0) {
				nonZeroPoolSizes.push_back(poolSize);
			}
		}

		VIVIUM_ASSERT(nonZeroPoolSizes.size() > 0, "No valid specifications passed");

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(nonZeroPoolSizes.size());
		poolInfo.pPoolSizes = nonZeroPoolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(manager.descriptorSets.specifications.size()); // TODO: should be fine?

		manager.descriptorPools.push_back(VK_NULL_HANDLE);

		VIVIUM_VK_CHECK(vkCreateDescriptorPool(engine.device,
			&poolInfo, nullptr, &manager.descriptorPools.back()), "Failed to create descriptor pool");

		// Begin populating descriptor set layouts and descriptor sets
		std::vector<VkDescriptorSetLayout> layouts = std::vector<VkDescriptorSetLayout>(manager.descriptorSets.specifications.size());
		std::vector<VkDescriptorSet> sets = std::vector<VkDescriptorSet>(manager.descriptorSets.specifications.size());

		uint64_t specificationIndex = 0;

		// Copy from specification layouts to layouts vector
		for (uint64_t i = 0; i < manager.descriptorSets.specifications.size(); i++) {
			layouts[i] = _getReference(manager, manager.descriptorSets.specifications[i].layout).layout;
		}

		// Create descriptor sets
		VkDescriptorSetAllocateInfo setAllocateInfo{};
		setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		setAllocateInfo.descriptorPool = manager.descriptorPools.back();
		setAllocateInfo.descriptorSetCount = static_cast<uint32_t>(manager.descriptorSets.specifications.size());
		setAllocateInfo.pSetLayouts = layouts.data();

		VIVIUM_VK_CHECK(vkAllocateDescriptorSets(engine.device,
			&setAllocateInfo, sets.data()), "Failed to allocate descriptor sets");

		// Copy from allocated sets to the descriptor resource
		// And count total uniforms

		uint64_t totalUniforms = 0;

		for (uint64_t i = 0; i < manager.descriptorSets.specifications.size(); i++) {
			manager.descriptorSets.resources[i].descriptorSet = sets[i];
			totalUniforms += manager.descriptorSets.specifications[i].uniforms.size();
		}

		// Populate descriptors
		std::vector<VkWriteDescriptorSet> descriptorWrites(totalUniforms);
		uint64_t descriptorWritesCount = 0;

		std::vector<VkDescriptorBufferInfo> bufferInfos;
		bufferInfos.reserve(totalUniforms);
		std::vector<VkDescriptorImageInfo> imageInfos;
		imageInfos.reserve(totalUniforms);

		for (uint64_t i = 0; i < manager.descriptorSets.specifications.size(); i++) {
			DescriptorSetSpecification const& specification = manager.descriptorSets.specifications[i];

			// Iterate each uniform in specification, and generate a descriptor set write
			for (uint64_t j = 0; j < specification.uniforms.size(); j++) {
				UniformBinding const& binding = manager.descriptorLayouts.specifications[specification.layout.referenceIndex].bindings[j];
				UniformData const& data = specification.uniforms[j];

				VkWriteDescriptorSet& write = descriptorWrites[descriptorWritesCount++];
				write.pBufferInfo = nullptr;
				write.pImageInfo = nullptr;
				write.pTexelBufferView = nullptr;

				switch (binding.type) {
				case UniformType::UNIFORM_BUFFER:
				case UniformType::STORAGE_BUFFER:
				{
					bufferInfos.push_back({});
					VkDescriptorBufferInfo& bufferInfo = bufferInfos.back();

					bufferInfo.buffer = _getReference(manager, data.bufferData.buffer).buffer;
					bufferInfo.offset = data.bufferData.offset;
					bufferInfo.range = data.bufferData.size;

					write.pBufferInfo = &bufferInfo;

					break;
				}
				case UniformType::TEXTURE:
				{
					imageInfos.push_back({});
					VkDescriptorImageInfo& imageInfo = imageInfos.back();

					Texture const& texture = _getReference(manager, data.textureData.texture);

					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = texture.view;
					imageInfo.sampler = texture.sampler;

					write.pImageInfo = &imageInfo;

					break;
				}
				case UniformType::FRAMEBUFFER:
				{
					imageInfos.push_back({});
					VkDescriptorImageInfo& imageInfo = imageInfos.back();

					Framebuffer const& framebuffer = _getReference(manager, data.framebufferData.framebuffer);

					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = framebuffer.view;
					imageInfo.sampler = framebuffer.sampler;

					write.pImageInfo = &imageInfo;

					break;
				}
				default:
					// TODO: use vk string
					VIVIUM_LOG(LogSeverity::FATAL, "Invalid uniform type {}", (int)binding.type);

					break;
				}

				write.descriptorType = _descriptorType(binding.type);
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = sets[i];
				write.dstBinding = binding.slot;
				write.dstArrayElement = 0; // TODO: support for arrays?
				write.descriptorCount = 1; // TODO: change when UBO arrays
			}
		}

		vkUpdateDescriptorSets(engine.device,
			static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	void _allocatePipelines(ResourceManager& manager, Engine& engine)
	{
		for (uint64_t i = 0; i < manager.pipelines.specifications.size(); i++) {
			PipelineSpecification const& specification = manager.pipelines.specifications[i];
			Pipeline& resource = manager.pipelines.resources[i];

			switch (specification.target) {
			case _RenderTarget::WINDOW:
				resource.renderPass = specification.windowPass; break;
			case _RenderTarget::FRAMEBUFFER:
				resource.renderPass = _getReference(manager, specification.framebuffer).renderPass; break;
			default:
				VIVIUM_LOG(LogSeverity::FATAL, "Invalid pipeline target, can't find render pass"); break;
			}

			std::vector<VkPipelineShaderStageCreateInfo> shaderStages(specification.shaders.size());

			for (uint32_t i = 0; i < specification.shaders.size(); i++) {
				ShaderReference const& shaderReference = specification.shaders[i];
				Shader const& shader = _getReference(manager, shaderReference);

				VkPipelineShaderStageCreateInfo shaderStageInfo{};
				shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shaderStageInfo.stage = static_cast<VkShaderStageFlagBits>(manager.shaders.specifications[shaderReference.referenceIndex].stage);
				shaderStageInfo.module = shader.shader;
				shaderStageInfo.pName = "main";

				shaderStages[i] = shaderStageInfo;
			}

			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(specification.bufferLayout.attributeDescriptions.size());
			vertexInputInfo.pVertexBindingDescriptions = &(specification.bufferLayout.bindingDescription);
			vertexInputInfo.pVertexAttributeDescriptions = specification.bufferLayout.attributeDescriptions.data();

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
			multisampling.rasterizationSamples = specification.sampleCount;

			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

			// TODO: possibly re-enable
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

			std::vector<VkDescriptorSetLayout> descriptorLayouts(specification.descriptorLayouts.size());

			for (uint32_t i = 0; i < descriptorLayouts.size(); i++) {
				descriptorLayouts[i] = _getReference(manager, specification.descriptorLayouts[i]).layout;
			}

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();
			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
			// TODO: dirty, better to just make the copy
			pipelineLayoutInfo.pPushConstantRanges = reinterpret_cast<const VkPushConstantRange*>(specification.pushConstants.data());
			pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(specification.pushConstants.size());

			VIVIUM_VK_CHECK(vkCreatePipelineLayout(
				engine.device,
				&pipelineLayoutInfo,
				nullptr,
				&resource.layout),
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
			pipelineInfo.layout = resource.layout;
			pipelineInfo.renderPass = resource.renderPass;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			pipelineInfo.basePipelineIndex = -1;

			VIVIUM_VK_CHECK(vkCreateGraphicsPipelines(
				engine.device,
				VK_NULL_HANDLE,
				1,
				&pipelineInfo,
				nullptr,
				&resource.pipeline
			), "Failed to create graphics pipeline");
		}
	}

	void allocateManager(ResourceManager& manager, Engine& engine)
	{
		if (!manager.hostBuffers.specifications.empty())
			_allocateBuffers(manager, engine, MemoryType::STAGING);
		if (!manager.deviceBuffers.specifications.empty())
			_allocateBuffers(manager, engine, MemoryType::DEVICE);
		if (!manager.textures.specifications.empty())
			_allocateTextures(manager, engine);
		if (!manager.framebuffers.specifications.empty())
			_allocateFramebuffers(manager, engine);
		if (!manager.descriptorLayouts.specifications.empty())
			_allocateDescriptorLayouts(manager, engine);
		if (!manager.shaders.specifications.empty())
			_allocateShaders(manager, engine);

		if (!manager.descriptorSets.specifications.empty())
			_allocateDescriptorSets(manager, engine);

		if (!manager.pipelines.specifications.empty())
			_allocatePipelines(manager, engine);

		manager.hostBuffers.specifications = {};
		manager.deviceBuffers.specifications = {};
		manager.textures.specifications = {};
		manager.framebuffers.specifications = {};
		manager.shaders.specifications = {};
		manager.descriptorLayouts.specifications = {};
		manager.descriptorSets.specifications = {};
		manager.pipelines.specifications = {};
	}

	void clearManagerReferences(ResourceManager& manager)
	{
		manager.hostBuffers.resources = {};
		manager.deviceBuffers.resources = {};
		manager.textures.resources = {};
		manager.framebuffers.resources = {};
		manager.shaders.resources = {};
		manager.descriptorLayouts.resources = {};
		manager.descriptorSets.resources = {};
		manager.pipelines.resources = {};
	}

	void submitResource(ResourceManager& manager, BufferReference* memory, MemoryType memoryType, const std::span<const BufferSpecification> specifications)
	{
		if ((static_cast<VkMemoryPropertyFlagBits>(memoryType) & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
			_addToResourceField(manager.hostBuffers, memory, specifications);

			for (uint64_t i = 0; i < specifications.size(); i++) {
				memory[i].memoryIndex = 0;
			}
		}
		else if ((static_cast<VkMemoryPropertyFlagBits>(memoryType) & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			_addToResourceField(manager.deviceBuffers, memory, specifications);

			for (uint64_t i = 0; i < specifications.size(); i++) {
				memory[i].memoryIndex = 1;
			}
		}
		else {
			VIVIUM_LOG(LogSeverity::FATAL, "Invalid memory type: {}", (uint64_t)memoryType);
		}
	}

	ResourceManager createManager()
	{
		return ResourceManager{};
	}

	void dropManager(ResourceManager& manager, Engine& engine) {
		// Free vulkan memory
		for (ResourceManager::DeviceMemoryHandle deviceMemoryHandle : manager.deviceMemoryHandles) {
			if (deviceMemoryHandle.mapping != nullptr)
				vkUnmapMemory(engine.device, deviceMemoryHandle.memory);

			vkFreeMemory(engine.device, deviceMemoryHandle.memory, nullptr);
		}

		for (VkDescriptorPool descriptorPool : manager.descriptorPools) {
			vkDestroyDescriptorPool(engine.device, descriptorPool, nullptr);
		}
	}

	void submitResource(ResourceManager& manager, TextureReference* memory, const std::span<const TextureSpecification> specifications)
	{
		_addToResourceField(manager.textures, memory, specifications);
	}

	void submitResource(ResourceManager& manager, DescriptorSetReference* memory, const std::span<const DescriptorSetSpecification> specifications)
	{
		_addToResourceField(manager.descriptorSets, memory, specifications);
	}

	void submitResource(ResourceManager& manager, DescriptorLayoutReference* memory, const std::span<const DescriptorLayoutSpecification> specifications)
	{
		_addToResourceField(manager.descriptorLayouts, memory, specifications);
	}

	void submitResource(ResourceManager& manager, ShaderReference* memory, const std::span<const ShaderSpecification> specifications)
	{
		_addToResourceField(manager.shaders, memory, specifications);
	}

	void submitResource(ResourceManager& manager, PipelineReference* memory, const std::span<const PipelineSpecification> specifications)
	{
		_addToResourceField(manager.pipelines, memory, specifications);
	}

	void submitResource(ResourceManager& manager, FramebufferReference* memory, const std::span<const FramebufferSpecification> specifications)
	{
		_addToResourceField(manager.framebuffers, memory, specifications);
	}

	Buffer& _getReference(ResourceManager& manager, BufferReference reference)
	{
		// Probably not best to switch on "0"
		switch (reference.memoryIndex) {
		case 0:
			return manager.hostBuffers.resources[reference.referenceIndex];
		case 1:
			return manager.deviceBuffers.resources[reference.referenceIndex];
		}

		VIVIUM_LOG(LogSeverity::FATAL, "Reference malformed");
	}

	Texture& _getReference(ResourceManager& manager, TextureReference reference)
	{
		return manager.textures.resources[reference.referenceIndex];
	}

	Framebuffer& _getReference(ResourceManager& manager, FramebufferReference reference)
	{
		return manager.framebuffers.resources[reference.referenceIndex];
	}

	Shader& _getReference(ResourceManager& manager, ShaderReference reference)
	{
		return manager.shaders.resources[reference.referenceIndex];
	}

	DescriptorLayout& _getReference(ResourceManager& manager, DescriptorLayoutReference reference)
	{
		return manager.descriptorLayouts.resources[reference.referenceIndex];
	}

	DescriptorSet& _getReference(ResourceManager& manager, DescriptorSetReference reference)
	{
		return manager.descriptorSets.resources[reference.referenceIndex];
	}

	Pipeline& _getReference(ResourceManager& manager, PipelineReference reference)
	{
		return manager.pipelines.resources[reference.referenceIndex];
	}
}