#pragma once

#include "../storage.h"

#include "primitives/buffer.h"
#include "primitives/descriptor_set.h"
#include "primitives/descriptor_layout.h"
#include "primitives/texture.h"
#include "primitives/memory_type.h"
#include "commands.h"

#include <atomic>

namespace Vivium {
	namespace ResourceManager {
		struct SharedTrackerData {
			std::atomic_uint32_t deviceMemoryAllocations;

			SharedTrackerData();
		};

		inline SharedTrackerData sharedTrackerData;

		namespace Static {
			template <Allocator::AllocatorType AllocatorType>
			struct Resource {
				template <typename ResourceType, typename SpecificationType>
				struct PreallocationData {
					std::vector<std::span<ResourceType>> resources;
					std::vector<SpecificationType> specifications;

					std::vector<SpecificationType> submit(Resource& resourceManager, const std::span<const SpecificationType> newSpecifications)
					{
						specifications.reserve(std::max(
							specifications.size() >> 1 + specifications.size(),
							specifications.size() + newSpecifications.size()
						));

						specifications.insert(specifications.end(), newSpecifications.begin(), newSpecifications.end());

						std::vector<ResourceType*> handles(newSpecifications.size());

						ResourceType* resourceBlock = reinterpret_cast<ResourceType*>(resourceManager.allocator.allocate(sizeof(ResourceType) * newSpecifications.size()));

						for (uint64_t i = 0; i < newSpecifications.size(); i++) {
							new (resourceBlock + i) ResourceType{};
							handles[i] = resourceBlock + i;
						}

						resources.push_back({ resourceBlock, newSpecifications.size() });
					}

					void clear() {
						resources = {};
						specifications = {};
					}
				};

				struct DeviceMemoryHandle {
					VkDeviceMemory memory;
					void* mapping;

					DeviceMemoryHandle()
						: memory(VK_NULL_HANDLE), mapping(nullptr) {}
				};

				AllocatorType allocator;

				PreallocationData<Buffer::Resource, Buffer::Specification> hostBuffers;
				PreallocationData<Buffer::Resource, Buffer::Specification> deviceBuffers;
				PreallocationData<Buffer::Dynamic::Resource, Buffer::Dynamic::Specification> dynamicHostBuffers;
				PreallocationData<Texture::Resource, Texture::Specification> textures;
				PreallocationData<DescriptorSet::Resource, DescriptorSet::Specification> descriptorSets;

				std::vector<DeviceMemoryHandle> deviceMemoryHandles;
				VkDescriptorPool descriptorPool;

				DeviceMemoryHandle allocateDeviceMemory(Engine::Handle engine, uint32_t memoryTypeBits, MemoryType memoryType, uint64_t size) {
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

				// 1.
				void allocateBuffers(MemoryType memoryType)
				{
					PreallocationData<Buffer::Resource, Buffer::Specification>& preallocationData;

					switch (memoryType) {
					case MemoryType::DEVICE: preallocationData = deviceBuffers; break;
					case MemoryType::UNIFORM: preallocationData = hostBuffers; break;
					default: VIVIUM_LOG(Log::FATAL, "Invalid memory type to allocate buffer to"); break;
					}

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
							Commands::createBuffer(&resource.buffer, specification.size, specification.usage, &memoryRequirements);
							// Calculate offset this buffer should be at in the device memory
							bufferOffsets[specificationIndex] = calculateAlignmentOffset(totalSize, memoryRequirements.size, memoryRequirements.alignment);
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

				void allocateDynamicBuffers();
				void allocateTextures();
				// 2.
				void allocateDescriptors();
			};

			template <Allocator::AllocatorType AllocatorType>
			using Handle = Resource<AllocatorType>*;
		}
	}
}