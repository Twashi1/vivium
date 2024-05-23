#include "resource_manager.h"

namespace Vivium {
	namespace ResourceManager {
		SharedTrackerData::SharedTrackerData()
			: deviceMemoryAllocations(0)
		{}

		namespace Static {
			Resource::DeviceMemoryHandle::DeviceMemoryHandle()
				: memory(VK_NULL_HANDLE), mapping(nullptr) {}

			Resource::DeviceMemoryHandle Resource::allocateDeviceMemory(Engine::Handle engine, uint32_t memoryTypeBits, MemoryType memoryType, uint64_t size) {
				DeviceMemoryHandle deviceMemoryHandle;

				VkMemoryAllocateInfo memoryAllocationInfo{};
				memoryAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				memoryAllocationInfo.allocationSize = size;
				memoryAllocationInfo.memoryTypeIndex = findMemoryType(
					engine,
					memoryTypeBits,
					static_cast<VkMemoryPropertyFlags>(memoryType)
				);

				VIVIUM_VK_CHECK(vkAllocateMemory(
					engine->device,
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
							engine->device,
							deviceMemoryHandle.memory,
							0,
							size,
							NULL,
							&deviceMemoryHandle.mapping
						),
						"Failed to map memory"
					);
				}

				sharedTrackerData.deviceMemoryAllocations++;

				deviceMemoryHandles.push_back(deviceMemoryHandle);

				return deviceMemoryHandle;
			}

			void Resource::allocateBuffers(Engine::Handle engine, MemoryType memoryType)
			{
				// TODO: edit this, so messy

				PreallocationData<Buffer::Resource, Buffer::Specification>* preallocationDataPointer;

				switch (memoryType) {
				case MemoryType::DEVICE: preallocationDataPointer = &deviceBuffers; break;
				case MemoryType::UNIFORM: preallocationDataPointer = &hostBuffers; break;
				default: VIVIUM_LOG(Log::FATAL, "Invalid memory type to allocate buffer to"); break;
				}

				PreallocationData<Buffer::Resource, Buffer::Specification>& preallocationData = *preallocationDataPointer;

				uint64_t specificationIndex = 0;
				uint64_t totalSize = 0;

				std::vector<uint64_t> bufferOffsets(preallocationData.specifications.size());
				uint32_t memoryTypeBits = 0;

				for (std::span<Buffer::Resource>& resourceSpan : preallocationData.resources) {
					for (uint64_t i = 0; i < resourceSpan.size(); i++) {
						Buffer::Resource& resource = resourceSpan[i];
						Buffer::Specification& specification = preallocationData.specifications[specificationIndex];
						VkMemoryRequirements memoryRequirements;

						resource.size = specification.size;
						resource.usage = specification.usage;

						// Create the VkBuffer and get the memory requirements
						Commands::createBuffer(engine, &resource.buffer, specification.size, specification.usage, &memoryRequirements, nullptr);
						// Calculate offset this buffer should be at in the device memory
						uint64_t resourceOffset = Math::nearestMultiple(totalSize, memoryRequirements.alignment);
						bufferOffsets[specificationIndex] = resourceOffset;
						totalSize = resourceOffset + memoryRequirements.size;
						
						// Include memory type bits
						memoryTypeBits |= memoryRequirements.memoryTypeBits;

						++specificationIndex;
					}
				}

				// Get some device memory for these buffers
				DeviceMemoryHandle deviceMemoryHandle = allocateDeviceMemory(
					engine,
					memoryTypeBits,
					memoryType,
					totalSize
				);

				// Bind buffers to memory
				specificationIndex = 0;

				for (std::span<Buffer::Resource>& resourceSpan : preallocationData.resources) {
					for (uint64_t i = 0; i < resourceSpan.size(); i++) {
						Buffer::Resource& resource = resourceSpan[i];

						vkBindBufferMemory(engine->device, resource.buffer, deviceMemoryHandle.memory, bufferOffsets[specificationIndex]);

						if (deviceMemoryHandle.mapping != nullptr)
							resource.mapping = reinterpret_cast<uint8_t*>(deviceMemoryHandle.mapping) + bufferOffsets[specificationIndex];

						++specificationIndex;
					}
				}
			}
			
			void Resource::allocateDynamicBuffers(Engine::Handle engine)
			{
				PreallocationData<Buffer::Dynamic::Resource, Buffer::Dynamic::Specification>& preallocationData = dynamicHostBuffers;

				uint64_t specificationIndex = 0;
				uint64_t totalSize = 0;

				std::vector<uint64_t> bufferOffsets(preallocationData.specifications.size());
				uint32_t memoryTypeBits = 0;

				for (std::span<Buffer::Dynamic::Resource>& resourceSpan : preallocationData.resources) {
					for (uint64_t i = 0; i < resourceSpan.size(); i++) {
						Buffer::Dynamic::Resource& resource = resourceSpan[i];
						Buffer::Dynamic::Specification& specification = preallocationData.specifications[specificationIndex];
						VkMemoryRequirements memoryRequirements;

						uint64_t totalBufferSize = 0;

						// Calculate suballocation alignment
						VkPhysicalDeviceProperties deviceProperties;
						vkGetPhysicalDeviceProperties(engine->physicalDevice, &deviceProperties);

						uint64_t suballocationAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;

						resource.suballocationSizes.resize(specification.suballocations.size());
						resource.suballocationOffsets.resize(specification.suballocations.size());

						for (uint64_t j = 0; j < specification.suballocations.size(); j++) {
							resource.suballocationSizes[j] = specification.suballocations[j];
							uint32_t suballocationOffset = Math::nearestMultiple(totalBufferSize, suballocationAlignment);
							resource.suballocationOffsets[j] = suballocationOffset;
							totalBufferSize = suballocationOffset + resource.suballocationSizes[j];
						}

						// Create the VkBuffer and get the memory requirements
						Commands::createBuffer(engine, &resource.buffer, totalBufferSize, specification.usage, &memoryRequirements, nullptr);
						// Calculate offset this buffer should be at in the device memory
						uint64_t bufferOffset = Math::nearestMultiple(totalSize, memoryRequirements.alignment);
						bufferOffsets[specificationIndex] = bufferOffset;
						totalSize = bufferOffset + memoryRequirements.size;
						// Include memory type bits
						memoryTypeBits |= memoryRequirements.memoryTypeBits;

						++specificationIndex;
					}
				}

				// Get some device memory for these buffers
				DeviceMemoryHandle deviceMemoryHandle = allocateDeviceMemory(
					engine,
					memoryTypeBits,
					MemoryType::DYNAMIC_UNIFORM,
					totalSize
				);

				// Bind buffers to memory
				specificationIndex = 0;

				for (std::span<Buffer::Dynamic::Resource>& resource_span : preallocationData.resources) {
					for (uint64_t i = 0; i < resource_span.size(); i++) {
						Buffer::Dynamic::Resource& resource = resource_span[i];

						vkBindBufferMemory(engine->device, resource.buffer, deviceMemoryHandle.memory, bufferOffsets[specificationIndex]);

						if (deviceMemoryHandle.mapping != nullptr)
							resource.mapping = reinterpret_cast<uint8_t*>(deviceMemoryHandle.mapping) + bufferOffsets[specificationIndex];

						++specificationIndex;
					}
				}
			}
			
			void Resource::allocateTextures(Engine::Handle engine)
			{
				// TODO: assert number of specifications passed is less than the number
				// of device memory blocks we can allocate,
				// otherwise we need to batch these staging buffers together (which we should do anyway)

				// For first transition, buffer copy, second transition
				std::array<VkCommandBuffer, 3> commandBuffers;
				VkCommandPool commandPool;

				// Create command pool
				Commands::createCommandPool(engine, &commandPool, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
				// Create command buffers
				Commands::createCommandBuffers(engine, commandPool, commandBuffers.data(), commandBuffers.size());

				// For storing temporary buffers, buffer image copy, pipeline barriers
				std::vector<VkBuffer> textureBuffers;
				std::vector<VkDeviceMemory> textureBufferMemories;
				std::vector<VkImageMemoryBarrier> textureBarriers;
				std::vector<VkBufferImageCopy> textureRegions;

				// Reserve space for texture temporaries
				uint64_t specificationCount = textures.specifications.size();
				textureBarriers.reserve(specificationCount * 2);
				textureBuffers.reserve(specificationCount);
				textureRegions.reserve(specificationCount);
				textureBufferMemories.reserve(specificationCount);

				// Calculate total size required
				uint64_t totalSize = 0;
				uint32_t memoryTypeBits = 0;
				std::vector<uint64_t> offsets(specificationCount);

				uint64_t specificationIndex = 0;

				for (std::span<Texture::Resource>& resourceSpan : textures.resources) {
					for (uint64_t i = 0; i < resourceSpan.size(); i++) {
						Texture::Resource& texture = resourceSpan[i];
						Texture::Specification& specification = textures.specifications[specificationIndex];

						Commands::createImage(
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
							engine->device,
							texture.image,
							&memoryRequirements
						);

						uint64_t resourceOffset = Math::nearestMultiple(totalSize, memoryRequirements.alignment);
						offsets[specificationIndex] = resourceOffset;
						totalSize = resourceOffset + memoryRequirements.size;

						memoryTypeBits |= memoryRequirements.memoryTypeBits;

						++specificationIndex;
					}
				}

				// Begin command buffers
				for (uint64_t i = 0; i < commandBuffers.size(); i++)
					Commands::beginCommandBuffer(
						commandBuffers[i],
						VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
					);

				// Create VkDeviceMemory
				DeviceMemoryHandle deviceMemoryHandle = allocateDeviceMemory(engine, memoryTypeBits, MemoryType::DEVICE, totalSize);

				specificationIndex = 0;

				std::vector<VkDeviceMemory> oneTimeStagingMemories;
				std::vector<VkBuffer> oneTimeStagingBuffers;

				for (std::span<Texture::Resource>& resourceSpan : textures.resources) {
					for (uint64_t i = 0; i < resourceSpan.size(); i++) {
						Texture::Resource& texture = resourceSpan[i];
						Texture::Specification& specification = textures.specifications[specificationIndex];

						// Bind image to memory
						VIVIUM_VK_CHECK(vkBindImageMemory(
							engine->device,
							texture.image,
							deviceMemoryHandle.memory,
							offsets[specificationIndex]
						), "Failed to bind image to memory");

						textureBarriers.push_back({});

						Commands::transitionImageLayout(
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

						Commands::createOneTimeStagingBuffer(
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

						Commands::moveBufferToImage(
							stagingBuffer,
							texture.image,
							commandBuffers[1],
							specification.width,
							specification.height,
							&textureRegions.back()
						);

						textureBarriers.push_back({});

						Commands::transitionImageLayout(
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

						++specificationIndex;
					}
				}

				// TODO: put on transfer queue once confirmed working
				Commands::endCommandBuffer(
					commandBuffers.data(),
					commandBuffers.size(),
					engine->graphicsQueue
				);
				// TODO: experiment with putting this wait until after creating view and sampler?
				vkQueueWaitIdle(engine->graphicsQueue);

				specificationIndex = 0;

				// TODO: maybe these don't need the image to be completely filled out beforehand?
				for (std::span<Texture::Resource>& resourceSpan : textures.resources) {
					for (uint64_t i = 0; i < resourceSpan.size(); i++) {
						Texture::Resource& texture = resourceSpan[i];
						Texture::Specification& specification = textures.specifications[specificationIndex];

						Commands::createView(engine, &texture.view, specification.imageFormat, texture.image, nullptr);
						Commands::createSampler(engine, &texture.sampler, specification.imageFilter, nullptr);

						++specificationIndex;
					}
				}

				// Wait until idle since we will free unnecessary resources now
				vkDeviceWaitIdle(engine->device);

				for (uint32_t i = 0; i < oneTimeStagingBuffers.size(); i++) {
					vkDestroyBuffer(engine->device, oneTimeStagingBuffers[i], nullptr);
					vkUnmapMemory(engine->device, oneTimeStagingMemories[i]);
					vkFreeMemory(engine->device, oneTimeStagingMemories[i], nullptr);
				}

				for (uint64_t i = 0; i < textureBuffers.size(); i++) {
					Commands::freeOneTimeStagingBuffer(
						engine,
						textureBuffers[i],
						textureBufferMemories[i]
					);
				}

				vkFreeCommandBuffers(
					engine->device,
					commandPool,
					static_cast<uint32_t>(commandBuffers.size()),
					commandBuffers.data()
				);

				vkDestroyCommandPool(engine->device, commandPool, nullptr);
			}

			void Resource::allocateFramebuffers(Engine::Handle engine)
			{
				uint64_t specificationIndex = 0;

				uint64_t totalMemoryRequired = 0;
				uint32_t memoryTypeBits = NULL;

				std::vector<uint64_t> imageMemoryLocations(framebuffers.specifications.size());

				// Create images and count memory requirements
				for (std::span<Framebuffer::Resource> resourceSpan : framebuffers.resources) {
					for (uint64_t resourceIndex = 0; resourceIndex < resourceSpan.size(); resourceIndex++) {
						const Framebuffer::Specification& specification = framebuffers.specifications[specificationIndex];
						Framebuffer::Resource& resource = resourceSpan[resourceIndex];

						resource.dimensions = specification.dimensions;
						resource.format = specification.format;

						Commands::createImage(
							engine,
							&resource.image,
							static_cast<uint32_t>(resource.dimensions.x), static_cast<uint32_t>(resource.dimensions.y),
							resource.format,
							VK_SAMPLE_COUNT_1_BIT,
							VK_IMAGE_LAYOUT_UNDEFINED,
							VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
							nullptr
						);

						VkMemoryRequirements requirements;
						vkGetImageMemoryRequirements(engine->device, resource.image, &requirements);

						VIVIUM_ASSERT(memoryTypeBits == requirements.memoryTypeBits || specificationIndex == 0, "Multiple memory types required?");

						memoryTypeBits = requirements.memoryTypeBits;

						uint64_t resourceOffset = Math::nearestMultiple(totalMemoryRequired, requirements.alignment);
						imageMemoryLocations[specificationIndex] = resourceOffset;
						totalMemoryRequired = resourceOffset + requirements.size;

						++specificationIndex;
					}
				}

				DeviceMemoryHandle memory = allocateDeviceMemory(engine, memoryTypeBits, MemoryType::DEVICE, totalMemoryRequired);

				specificationIndex = 0;

				for (std::span<Framebuffer::Resource> resourceSpan : framebuffers.resources) {
					for (uint64_t resourceIndex = 0; resourceIndex < resourceSpan.size(); resourceIndex++) {
						const Framebuffer::Specification& specification = framebuffers.specifications[specificationIndex];
						Framebuffer::Resource& resource = resourceSpan[resourceIndex];

						VIVIUM_VK_CHECK(vkBindImageMemory(
							engine->device,
							resource.image,
							memory.memory,
							imageMemoryLocations[specificationIndex]
						), "Failed to bind image memory");

						Commands::createView(engine, &resource.view, resource.format, resource.image, nullptr);
						// TODO: customiseable filter
						Commands::createSampler(engine, &resource.sampler, Texture::Filter::LINEAR, nullptr);
				
						// Creating render pass
						VkAttachmentDescription colorAttachment{};
						colorAttachment.format = static_cast<VkFormat>(resource.format);
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

						VIVIUM_VK_CHECK(vkCreateRenderPass(engine->device, &renderPassInfo, nullptr, &resource.renderPass), "Failed render pass");

						VkFramebufferCreateInfo framebufferInfo{};
						framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
						framebufferInfo.renderPass = resource.renderPass;
						framebufferInfo.attachmentCount = 1;
						framebufferInfo.pAttachments = &resource.view;
						framebufferInfo.width = static_cast<uint32_t>(resource.dimensions.x);
						framebufferInfo.height = static_cast<uint32_t>(resource.dimensions.y);
						framebufferInfo.layers = 1;

						VIVIUM_VK_CHECK(vkCreateFramebuffer(engine->device, &framebufferInfo, nullptr, &resource.framebuffer), "Failed create fb");

						++specificationIndex;
					}
				}
			}

			void Resource::allocateDescriptors(Engine::Handle engine)
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
					// NOTE: this is slow O(n^2), could use std::unordered_set, but doubt it would be faster
					std::vector<DescriptorLayout::Handle> seenLayouts;

					for (const DescriptorSet::Specification& specification : descriptorSets.specifications) {
						if (std::find(seenLayouts.begin(), seenLayouts.end(), specification.layout) == seenLayouts.end())
							seenLayouts.push_back(specification.layout);
						else continue;

						// Required for this to work
						for (const Uniform::Binding& binding : specification.layout->bindings) {
							switch (binding.type) {
							case Uniform::Type::UNIFORM_BUFFER:
								poolSizeCounts[0].descriptorCount++; break;
							case Uniform::Type::TEXTURE:
							case Uniform::Type::FRAMEBUFFER:
								poolSizeCounts[1].descriptorCount++; break;
							case Uniform::Type::STORAGE_BUFFER:
								poolSizeCounts[2].descriptorCount++; break;
							case Uniform::Type::DYNAMIC_UNIFORM_BUFFER:
								poolSizeCounts[3].descriptorCount++; break;
							default:
								VIVIUM_LOG(Log::FATAL, "Invalid uniform type"); break;
							}
						}
					}
				}

				std::vector<VkDescriptorPoolSize> nonZeroPoolSizes;
				nonZeroPoolSizes.reserve(4);

				for (VkDescriptorPoolSize poolSize : poolSizeCounts)
					if (poolSize.descriptorCount != 0)
						nonZeroPoolSizes.push_back(poolSize);

				VIVIUM_ASSERT(nonZeroPoolSizes.size() > 0, "No valid specifications passed");

				VkDescriptorPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				poolInfo.poolSizeCount = static_cast<uint32_t>(nonZeroPoolSizes.size());
				poolInfo.pPoolSizes = nonZeroPoolSizes.data();
				poolInfo.maxSets = static_cast<uint32_t>(descriptorSets.specifications.size()); // TODO: should be fine?

				VIVIUM_VK_CHECK(vkCreateDescriptorPool(engine->device,
					&poolInfo, nullptr, &descriptorPool), "Failed to create descriptor pool");

				// Begin populating descriptor set layouts and descriptor sets
				std::vector<VkDescriptorSetLayout> layouts = std::vector<VkDescriptorSetLayout>(descriptorSets.specifications.size());
				std::vector<VkDescriptorSet> sets = std::vector<VkDescriptorSet>(descriptorSets.specifications.size());

				uint64_t specificationIndex = 0;

				// Copy from specification layouts to layouts vector
				for (uint64_t i = 0; i < descriptorSets.specifications.size(); i++) {
					layouts[i] = descriptorSets.specifications[i].layout->layout;
				}

				// Create descriptor sets
				VkDescriptorSetAllocateInfo setAllocateInfo{};
				setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				setAllocateInfo.descriptorPool = descriptorPool;
				setAllocateInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSets.specifications.size());
				setAllocateInfo.pSetLayouts = layouts.data();

				VIVIUM_VK_CHECK(vkAllocateDescriptorSets(engine->device,
					&setAllocateInfo, sets.data()), "Failed to allocate descriptor sets");

				specificationIndex = 0;

				// Copy from allocated sets to the descriptor resource
				for (std::span<DescriptorSet::Resource>& resourceSpan : descriptorSets.resources) {
					for (uint64_t i = 0; i < resourceSpan.size(); i++) {
						resourceSpan[i].descriptorSet = sets[specificationIndex];

						++specificationIndex;
					}
				}

				// Populate descriptors
				uint64_t totalUniforms = 0;

				// Quick iteration to count total uniforms
				for (uint64_t i = 0; i < descriptorSets.specifications.size(); i++) {
					totalUniforms += descriptorSets.specifications[i].uniforms.size();
				}

				std::vector<VkWriteDescriptorSet> descriptorWrites(totalUniforms);
				uint64_t descriptorWritesCount = 0;

				std::vector<VkDescriptorBufferInfo> bufferInfos;
				bufferInfos.reserve(totalUniforms);
				std::vector<VkDescriptorImageInfo> imageInfos;
				imageInfos.reserve(totalUniforms);

				for (uint64_t i = 0; i < descriptorSets.specifications.size(); i++) {
					const DescriptorSet::Specification& specification = descriptorSets.specifications[i];

					// Iterate each uniform in specification, and generate a descriptor set write
					for (uint64_t j = 0; j < specification.uniforms.size(); j++) {
						const Uniform::Binding& binding = specification.layout->bindings[j];
						const Uniform::Data& data = specification.uniforms[j];

						VkWriteDescriptorSet& write = descriptorWrites[descriptorWritesCount++];
						write.pBufferInfo = nullptr;
						write.pImageInfo = nullptr;
						write.pTexelBufferView = nullptr;

						switch (binding.type) {
						case Uniform::Type::UNIFORM_BUFFER:
						case Uniform::Type::STORAGE_BUFFER:
						{
							bufferInfos.push_back({});
							VkDescriptorBufferInfo& bufferInfo = bufferInfos.back();

							bufferInfo.buffer = data.bufferData.buffer->buffer;
							bufferInfo.offset = data.bufferData.offset;
							bufferInfo.range = data.bufferData.size;

							write.pBufferInfo = &bufferInfo;

							break;
						}
						case Uniform::Type::DYNAMIC_UNIFORM_BUFFER:
						{
							bufferInfos.push_back({});
							VkDescriptorBufferInfo& bufferInfo = bufferInfos.back();

							bufferInfo.buffer = data.dynamicBufferData.buffer->buffer;
							bufferInfo.offset = data.dynamicBufferData.offset;
							bufferInfo.range = data.dynamicBufferData.size;

							write.pBufferInfo = &bufferInfo;

							break;
						}
						case Uniform::Type::TEXTURE:
						{
							imageInfos.push_back({});
							VkDescriptorImageInfo& imageInfo = imageInfos.back();

							imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							imageInfo.imageView = data.textureData.texture->view;
							imageInfo.sampler = data.textureData.texture->sampler;

							write.pImageInfo = &imageInfo;

							break;
						}
						case Uniform::Type::FRAMEBUFFER:
						{
							imageInfos.push_back({});
							VkDescriptorImageInfo& imageInfo = imageInfos.back();

							imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							imageInfo.imageView = data.framebufferData.framebuffer->view;
							imageInfo.sampler = data.framebufferData.framebuffer->sampler;

							write.pImageInfo = &imageInfo;

							break;
						}
						default:
							// TODO: use vk string
							VIVIUM_LOG(Log::FATAL, "Invalid uniform type {}", (int)binding.type);

							break;
						}

						write.descriptorType = Uniform::descriptorType(binding.type);
						write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						write.dstSet = sets[i];
						write.dstBinding = binding.slot;
						write.dstArrayElement = 0; // TODO: support for arrays?
						write.descriptorCount = 1; // TODO: change when UBO arrays
					}
				}

				vkUpdateDescriptorSets(engine->device,
					static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
			
			void Resource::allocatePipelines(Engine::Handle engine)
			{
				uint64_t specificationIndex = 0;

				for (const std::span<Pipeline::Resource>& resourceSpan : pipelines.resources) {
					for (uint64_t resourceIndex = 0; resourceIndex < resourceSpan.size(); resourceIndex++) {
						const Pipeline::Specification& specification = pipelines.specifications[specificationIndex++];

						Pipeline::Resource& resource = resourceSpan[resourceIndex];
						switch (specification.target) {
						case Pipeline::Target::WINDOW:
							resource.renderPass = specification.engine->renderPass; break;
						case Pipeline::Target::FRAMEBUFFER:
							resource.renderPass = specification.framebuffer->renderPass; break;
						default:
							VIVIUM_LOG(Log::FATAL, "Invalid pipeline target, can't find render pass"); break;
						}
						
						std::vector<VkPipelineShaderStageCreateInfo> shaderStages(specification.shaders.size());

						for (uint32_t i = 0; i < specification.shaders.size(); i++) {
							Shader::Handle shader = specification.shaders[i];

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
							descriptorLayouts[i] = specification.descriptorLayouts[i]->layout;
						}

						VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
						pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
						pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();
						pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
						// TODO: dirty, better to just make the copy
						pipelineLayoutInfo.pPushConstantRanges = reinterpret_cast<const VkPushConstantRange*>(specification.pushConstants.data());
						pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(specification.pushConstants.size());

						VIVIUM_VK_CHECK(vkCreatePipelineLayout(
							engine->device,
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
							engine->device,
							VK_NULL_HANDLE,
							1,
							&pipelineInfo,
							nullptr,
							&resource.pipeline
						), "Failed to create graphics pipeline");
					}
				}
			}

			Resource::Resource()
				// TODO: editable block size
				: descriptorPool(VK_NULL_HANDLE) {}

			void Resource::allocate(Engine::Handle engine) {
				if (!hostBuffers.specifications.empty())
					allocateBuffers(engine, MemoryType::STAGING);
				if (!deviceBuffers.specifications.empty())
					allocateBuffers(engine, MemoryType::DEVICE);
				if (!dynamicHostBuffers.specifications.empty())
					allocateDynamicBuffers(engine);
				if (!textures.specifications.empty())
					allocateTextures(engine);
				if (!framebuffers.specifications.empty())
					allocateFramebuffers(engine);

				if (!descriptorSets.specifications.empty())
					allocateDescriptors(engine);

				if (!pipelines.specifications.empty())
					allocatePipelines(engine);

				hostBuffers.clear();
				deviceBuffers.clear();
				dynamicHostBuffers.clear();
				textures.clear();
				framebuffers.clear();
				descriptorSets.clear();
				pipelines.clear();
			}
			
			void Resource::drop(Engine::Handle engine)
			{
				// Free vulkan memory
				for (DeviceMemoryHandle deviceMemoryHandle : deviceMemoryHandles) {
					if (deviceMemoryHandle.mapping != nullptr)
						vkUnmapMemory(engine->device, deviceMemoryHandle.memory);

					vkFreeMemory(engine->device, deviceMemoryHandle.memory, nullptr);
				}

				sharedTrackerData.deviceMemoryAllocations -= static_cast<uint32_t>(deviceMemoryHandles.size());

				// Free descriptor pool
				vkDestroyDescriptorPool(engine->device, descriptorPool, nullptr);

				// Free resources
				allocationContext.storage.free();
			}

			std::vector<Buffer::PromisedHandle> Resource::submit(MemoryType memoryType, const std::span<const Buffer::Specification> specifications)
			{
				switch (memoryType) {
				case MemoryType::STAGING:
					return hostBuffers.submit(&allocationContext.storage, specifications); break;
				case MemoryType::DEVICE:
					return deviceBuffers.submit(&allocationContext.storage, specifications); break;
				default:
					VIVIUM_LOG(Log::FATAL, "Invalid memory type for submit"); break;
				}

				return {};
			}

			std::vector<Buffer::Dynamic::PromisedHandle> Resource::submit(const std::span<const Buffer::Dynamic::Specification> specifications)
			{
				return dynamicHostBuffers.submit(&allocationContext.storage, specifications);
			}

			std::vector<Texture::PromisedHandle> Resource::submit(const std::span<const Texture::Specification> specifications)
			{
				return textures.submit(&allocationContext.storage, specifications);
			}
			
			std::vector<DescriptorSet::PromisedHandle> Resource::submit(const std::span<const DescriptorSet::Specification> specifications)
			{
				return descriptorSets.submit(&allocationContext.storage, specifications);
			}

			std::vector<Pipeline::PromisedHandle> Resource::submit(const std::span<const Pipeline::Specification> specifications)
			{
				return pipelines.submit(&allocationContext.storage, specifications);
			}

			std::vector<Framebuffer::PromisedHandle> Resource::submit(const std::span<const Framebuffer::Specification> specifications)
			{
				return framebuffers.submit(&allocationContext.storage, specifications);
			}

			void* _vulkanAllocation(void* userData, uint64_t size, uint64_t alignment, VkSystemAllocationScope allocationScope)
			{
				AllocationContext* context = reinterpret_cast<AllocationContext*>(userData);

				void* allocation = context->storage.allocate(4, alignment, 4 + size + context->nextAllocationMetadataSize);
				// Include size of vulkan reosurce (before pointer, in header section)
				*(reinterpret_cast<uint32_t*>(allocation) - 1) = size;
				// Include size of metadata (after vulkan resource, in metadata section)
				*reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(allocation) + size) = context->nextAllocationMetadataSize;

				VIVIUM_LOG(Log::DEBUG, "Allocated resource at {}", allocation);

				return allocation;
			}

			void* _vulkanReallocation(void* userData, void* originalData, uint64_t size, uint64_t alignment, VkSystemAllocationScope allocationScope)
			{
				// TODO: actually attempt reallocation sometimes

				// Create a deletion context for the free
				AllocationContext* context = reinterpret_cast<AllocationContext*>(userData);
				static_assert(sizeof(DeletionContext) == sizeof(DestructorFunction));
				DeletionContext* deletionContext = reinterpret_cast<DeletionContext*>(&(context->destructor));

				if (size == 0) {
					VIVIUM_DEBUG_LOG("Reallocating with size 0, resorting to free");

					_vulkanFree(deletionContext, originalData);

					return NULL;
				}

				if (originalData == NULL) {
					VIVIUM_DEBUG_LOG("Delegating empty original reallocation to allocation");

					return _vulkanAllocation(userData, size, alignment, allocationScope);
				}

				void* allocation = context->storage.allocate(4, alignment, size + context->nextAllocationMetadataSize);

				VIVIUM_LOG(Log::DEBUG, "Allocated resource at {}", allocation);

				// Yikes
				uint32_t originalAllocationSize = *(reinterpret_cast<uint32_t*>(allocation) - 1);
				// Double yikes
				uint32_t originalMetadataSize = *reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(allocation) + originalAllocationSize);

				// Copy vulkan data
				std::memcpy(allocation, originalData, originalAllocationSize);
				// Copy metadata (assuming byte copy is enough)
				std::memcpy(
					reinterpret_cast<uint8_t*>(allocation) + size + 4,
					reinterpret_cast<uint8_t*>(originalData) + originalAllocationSize + 4, originalMetadataSize);
				// Set old metadata to 0 (in case it has some complexity)
				// TODO: parameterise this and only perform when necessary? insignificant performance difference
				std::memset(reinterpret_cast<uint8_t*>(originalData) + originalAllocationSize + 4, 0, originalMetadataSize);

				_vulkanFree(deletionContext, originalData);

				return allocation;
			}

			void _vulkanFree(void* userData, void* memory)
			{
				if (memory == NULL) return;

				DeletionContext* context = reinterpret_cast<DeletionContext*>(userData);

				VIVIUM_LOG(Log::DEBUG, "Deleting resource at {}", memory);

				uint32_t originalAllocationSize = *(reinterpret_cast<uint32_t*>(memory) - 1);

				context->destructor(reinterpret_cast<uint8_t*>(memory) + originalAllocationSize + 4);
			}

			void allocate(Engine::Handle engine, Handle handle) {
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);

				handle->allocate(engine);
			}

			std::vector<Buffer::PromisedHandle> submit(Handle handle, MemoryType memoryType, const std::span<const Buffer::Specification> specifications)
			{
				return handle->submit(memoryType, specifications);
			}

			std::vector<Buffer::Dynamic::PromisedHandle> submit(Handle handle, const std::span<const Buffer::Dynamic::Specification> specifications)
			{
				return handle->submit(specifications);
			}

			std::vector<Texture::PromisedHandle> submit(Handle handle, const std::span<const Texture::Specification> specifications)
			{
				return handle->submit(specifications);
			}

			std::vector<DescriptorSet::PromisedHandle> submit(Handle handle, const std::span<const DescriptorSet::Specification> specifications)
			{
				return handle->submit(specifications);
			}

			std::vector<Pipeline::PromisedHandle> submit(Handle handle, const std::span<const Pipeline::Specification> specifications)
			{
				return handle->submit(specifications);
			}

			std::vector<Framebuffer::PromisedHandle> submit(Handle handle, const std::span<const Framebuffer::Specification> specifications)
			{
				return handle->submit(specifications);
			}
		}
	}
}
