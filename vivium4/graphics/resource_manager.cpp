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
						Commands::createBuffer(engine, &resource.buffer, specification.size, specification.usage, &memoryRequirements);
						// Calculate offset this buffer should be at in the device memory
						bufferOffsets[specificationIndex] = Math::calculateAlignmentOffset(totalSize, memoryRequirements.size, memoryRequirements.alignment);
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
							resource.suballocationOffsets[j] = Math::calculateAlignmentOffset(totalBufferSize, resource.suballocationSizes[j], suballocationAlignment);
						}

						// Create the VkBuffer and get the memory requirements
						Commands::createBuffer(engine, &resource.buffer, totalBufferSize, specification.usage, &memoryRequirements);
						// Calculate offset this buffer should be at in the device memory
						bufferOffsets[specificationIndex] = Math::calculateAlignmentOffset(totalSize, memoryRequirements.size, memoryRequirements.alignment);
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

				PreallocationData<Texture::Resource, Texture::Specification>& preallocationData = textures;

				// Reserve space for texture temporaries
				uint64_t specificationCount = preallocationData.specifications.size();
				textureBarriers.reserve(specificationCount * 2);
				textureBuffers.reserve(specificationCount);
				textureRegions.reserve(specificationCount);
				textureBufferMemories.reserve(specificationCount);

				// Calculate total size required
				uint64_t totalSize = 0;
				uint32_t memoryTypeBits = 0;
				std::vector<uint64_t> offsets(specificationCount);

				uint64_t specificationIndex = 0;

				for (std::span<Texture::Resource>& resourceSpan : preallocationData.resources) {
					for (uint64_t i = 0; i < resourceSpan.size(); i++) {
						Texture::Resource& texture = resourceSpan[i];
						Texture::Specification& specification = preallocationData.specifications[specificationIndex];

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
							VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
						);

						VkMemoryRequirements memoryRequirements;

						vkGetImageMemoryRequirements(
							engine->device,
							texture.image,
							&memoryRequirements
						);

						offsets[specificationIndex] = Math::calculateAlignmentOffset(
							totalSize,
							memoryRequirements.size,
							memoryRequirements.alignment
						);

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

				for (std::span<Texture::Resource>& resourceSpan : preallocationData.resources) {
					for (uint64_t i = 0; i < resourceSpan.size(); i++) {
						Texture::Resource& texture = resourceSpan[i];
						Texture::Specification& specification = preallocationData.specifications[specificationIndex];

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
							specification.sizeBytes,
							&stagingMapping
						);

						oneTimeStagingBuffers.push_back(stagingBuffer);
						oneTimeStagingMemories.push_back(stagingMemory);

						std::memcpy(stagingMapping, specification.data, specification.sizeBytes);

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
				for (std::span<Texture::Resource>& resourceSpan : preallocationData.resources) {
					for (uint64_t i = 0; i < resourceSpan.size(); i++) {
						Texture::Resource& texture = resourceSpan[i];
						Texture::Specification& specification = preallocationData.specifications[specificationIndex];

						Commands::createView(engine, &texture.view, specification.imageFormat, texture.image);
						Commands::createSampler(engine, &texture.sampler, specification.imageFilter);

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
					commandBuffers.size(),
					commandBuffers.data()
				);

				vkDestroyCommandPool(engine->device, commandPool, nullptr);
			}

			void Resource::allocateDescriptors(Engine::Handle engine)
			{
				PreallocationData<DescriptorSet::Resource, DescriptorSet::Specification>& preallocationData = descriptorSets;

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

					for (const DescriptorSet::Specification& specification : preallocationData.specifications) {
						if (std::find(seenLayouts.begin(), seenLayouts.end(), specification.layout) == seenLayouts.end())
							seenLayouts.push_back(specification.layout);
						else continue;

						for (const Uniform::Binding& binding : specification.layout->bindings) {
							switch (binding.type) {
							case Uniform::Type::UNIFORM_BUFFER:
								poolSizeCounts[0].descriptorCount++; break;
							case Uniform::Type::TEXTURE:
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
				poolInfo.poolSizeCount = nonZeroPoolSizes.size();
				poolInfo.pPoolSizes = nonZeroPoolSizes.data();
				poolInfo.maxSets = preallocationData.specifications.size(); // TODO: should be fine?

				VIVIUM_VK_CHECK(vkCreateDescriptorPool(engine->device,
					&poolInfo, nullptr, &descriptorPool), "Failed to create descriptor pool");

				// Begin populating descriptor set layouts and descriptor sets
				std::vector<VkDescriptorSetLayout> layouts = std::vector<VkDescriptorSetLayout>(preallocationData.specifications.size());
				std::vector<VkDescriptorSet> sets = std::vector<VkDescriptorSet>(preallocationData.specifications.size());

				uint64_t specificationIndex = 0;

				// Copy from specification layouts to layouts vector
				for (uint64_t i = 0; i < preallocationData.specifications.size(); i++) {
					layouts[i] = preallocationData.specifications[i].layout->layout;
				}

				// Create descriptor sets
				VkDescriptorSetAllocateInfo setAllocateInfo{};
				setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				setAllocateInfo.descriptorPool = descriptorPool;
				setAllocateInfo.descriptorSetCount = preallocationData.specifications.size();
				setAllocateInfo.pSetLayouts = layouts.data();

				VIVIUM_VK_CHECK(vkAllocateDescriptorSets(engine->device,
					&setAllocateInfo, sets.data()), "Failed to allocate descriptor sets");

				specificationIndex = 0;

				// Copy from allocated sets to the descriptor resource
				for (std::span<DescriptorSet::Resource>& resourceSpan : preallocationData.resources) {
					for (uint64_t i = 0; i < resourceSpan.size(); i++) {
						resourceSpan[i].descriptorSet = sets[specificationIndex];

						++specificationIndex;
					}
				}

				// Populate descriptors
				uint32_t totalUniforms = 0;

				// Quick iteration to count total uniforms
				for (uint32_t i = 0; i < preallocationData.specifications.size(); i++) {
					totalUniforms += preallocationData.specifications[i].uniforms.size();
				}

				std::vector<VkWriteDescriptorSet> descriptorWrites(totalUniforms);
				uint64_t descriptorWritesCount = 0;

				std::vector<VkDescriptorBufferInfo> bufferInfos;
				bufferInfos.reserve(totalUniforms);
				std::vector<VkDescriptorImageInfo> imageInfos;
				imageInfos.reserve(totalUniforms);

				for (uint32_t i = 0; i < preallocationData.specifications.size(); i++) {
					const DescriptorSet::Specification& specification = preallocationData.specifications[i];

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
							write.descriptorType = static_cast<VkDescriptorType>(binding.type);

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
							write.descriptorType = static_cast<VkDescriptorType>(binding.type);

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
							write.descriptorType = static_cast<VkDescriptorType>(binding.type);

							break;
						}
						default:
							// TODO: use vk string
							VIVIUM_LOG(Log::FATAL, "Invalid uniform type {}", (int)binding.type);

							break;
						}

						write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						write.dstSet = sets[i];
						write.dstBinding = binding.slot;
						write.dstArrayElement = 0; // TODO: support for arrays?
						write.descriptorCount = 1; // TODO: change when UBO arrays
					}
				}

				vkUpdateDescriptorSets(engine->device,
					descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
			}
			
			Resource::Resource()
				// TODO: editable block size
				: descriptorPool(VK_NULL_HANDLE) {}
			
			// TODO: proper identifier of it being null?
			bool Resource::isNull() const {
				return false;
			}

			void Resource::allocate(Engine::Handle engine) {
				if (!hostBuffers.specifications.empty())
					allocateBuffers(engine, MemoryType::STAGING);
				if (!deviceBuffers.specifications.empty())
					allocateBuffers(engine, MemoryType::DEVICE);
				if (!dynamicHostBuffers.specifications.empty())
					allocateDynamicBuffers(engine);
				if (!textures.specifications.empty())
					allocateTextures(engine);

				if (!descriptorSets.specifications.empty())
					allocateDescriptors(engine);

				hostBuffers.clear();
				deviceBuffers.clear();
				dynamicHostBuffers.clear();
				textures.clear();
				descriptorSets.clear();
			}
			
			void Resource::drop(Engine::Handle engine)
			{
				// Free vulkan memory
				for (DeviceMemoryHandle deviceMemoryHandle : deviceMemoryHandles) {
					if (deviceMemoryHandle.mapping != nullptr)
						vkUnmapMemory(engine->device, deviceMemoryHandle.memory);

					vkFreeMemory(engine->device, deviceMemoryHandle.memory, nullptr);
				}

				sharedTrackerData.deviceMemoryAllocations -= deviceMemoryHandles.size();

				// Free descriptor pool
				vkDestroyDescriptorPool(engine->device, descriptorPool, nullptr);

				// Free resources
				resourceAllocator.free();
			}

			std::vector<Buffer::Handle> Resource::submit(MemoryType memoryType, const std::span<const Buffer::Specification> specifications)
			{
				switch (memoryType) {
				case MemoryType::STAGING:
					return hostBuffers.submit(&resourceAllocator, specifications); break;
				case MemoryType::DEVICE:
					return deviceBuffers.submit(&resourceAllocator, specifications); break;
				default:
					VIVIUM_LOG(Log::FATAL, "Invalid memory type for submit"); break;
				}

				return {};
			}

			std::vector<Buffer::Dynamic::Handle> Resource::submit(const std::span<const Buffer::Dynamic::Specification> specifications)
			{
				return dynamicHostBuffers.submit(&resourceAllocator, specifications);
			}

			std::vector<Texture::Handle> Resource::submit(const std::span<const Texture::Specification> specifications)
			{
				return textures.submit(&resourceAllocator, specifications);
			}
			
			std::vector<DescriptorSet::Handle> Resource::submit(const std::span<const DescriptorSet::Specification> specifications)
			{
				return descriptorSets.submit(&resourceAllocator, specifications);
			}
			
			void allocate(Engine::Handle engine, Handle handle) {
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine);
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(handle);

				handle->allocate(engine);
			}

			std::vector<Buffer::Handle> submit(Handle handle, MemoryType memoryType, const std::span<const Buffer::Specification> specifications)
			{
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(handle);

				return handle->submit(memoryType, specifications);
			}

			std::vector<Buffer::Dynamic::Handle> submit(Handle handle, const std::span<const Buffer::Dynamic::Specification> specifications)
			{
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(handle);

				return handle->submit(specifications);
			}

			std::vector<Texture::Handle> submit(Handle handle, const std::span<const Texture::Specification> specifications)
			{
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(handle);

				return handle->submit(specifications);
			}

			std::vector<DescriptorSet::Handle> submit(Handle handle, const std::span<const DescriptorSet::Specification> specifications)
			{
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(handle);

				return handle->submit(specifications);
			}
		}
	}
}
