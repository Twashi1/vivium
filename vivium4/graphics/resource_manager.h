#pragma once

#include "../storage.h"

#include "primitives/buffer.h"
#include "primitives/descriptor_set.h"
#include "primitives/descriptor_layout.h"
#include "primitives/texture.h"
#include "primitives/memory_type.h"
#include "primitives/pipeline.h"
#include "commands.h"
#include "../math/math.h"

#include <atomic>

namespace Vivium {
	namespace ResourceManager {
		struct _SharedTrackerData {
			std::atomic_uint32_t deviceMemoryAllocations;

			_SharedTrackerData();
		};

		inline _SharedTrackerData _sharedTrackerData;

		namespace Static {
			typedef void(*DestructorFunction)(void* object);

			struct AllocationContext {
				DestructorFunction destructor;
				
				Allocator::Static::Pool* storage;
				uint16_t nextAllocationMetadataSize;
			};

			struct DeletionContext {
				DestructorFunction destructor;
			};

			struct AllocationHeader {
				uint32_t allocationSize;
				uint16_t metadataSize;
			};

			AllocationHeader* _getHeader(void* viviumResource);
			void* _getMetadata(void* viviumResource, AllocationHeader* header);

			void* _vulkanAllocation(void* userData, uint64_t size, uint64_t alignment, VkSystemAllocationScope allocationScope);
			void* _vulkanReallocation(void* userData, void* originalData, uint64_t size, uint64_t alignment, VkSystemAllocationScope allocationScope);
			void _vulkanFree(void* userData, void* memory);

			uint64_t _calculateAlignedOffset(uint64_t* offset, uint64_t alignment, uint64_t size);

			struct Reference {
				uint64_t specificationIndex;
			};

			struct Resource {
				struct DeviceMemoryHandle {
					VkDeviceMemory memory;
					void* mapping;

					DeviceMemoryHandle();
				};

				std::vector<Buffer::Specification> hostBuffers;
				std::vector<Buffer::Specification> deviceBuffers;
				std::vector<Buffer::Dynamic::Specification> dynamicHostBuffers;
				std::vector<Texture::Specification> textures;
				std::vector<Framebuffer::Specification> framebuffers;
				std::vector<DescriptorSet::Specification> descriptors;
				std::vector<Pipeline::Specification> pipelines;

				Buffer::Handle* hostBufferHandles;
				Buffer::Handle* deviceBufferHandles;
				Buffer::Dynamic::Handle* dynamicBufferHandles;
				Texture::Handle* textureHandles;
				Framebuffer::Handle* framebufferHandles;
				DescriptorSet::Handle* descriptorHandles;
				Pipeline::Handle* pipelineHandles;

				std::vector<DeviceMemoryHandle> deviceMemoryHandles;
				std::vector<VkDescriptorPool> descriptorPools;

				AllocationContext allocationContext;
				DeletionContext deletionContext;
				VkAllocationCallbacks allocationCallbacks;
				// TODO: in future, make this customiseable allocator
				Allocator::Static::Pool resourceAllocator;

				DeviceMemoryHandle allocateDeviceMemory(Engine::Handle engine, uint32_t memoryTypeBits, MemoryType memoryType, uint64_t size);

				// 1.
				void allocateBuffers(Engine::Handle engine, MemoryType memoryType);
				void allocateDynamicBuffers(Engine::Handle engine);
				void allocateTextures(Engine::Handle engine);
				void allocateFramebuffers(Engine::Handle engine);

				// 2.
				void allocateDescriptors(Engine::Handle engine);

				// 3.
				void allocatePipelines(Engine::Handle engine);

				void drop(Buffer::Handle buffer, Engine::Handle engine);
				void drop(Buffer::Dynamic::Handle buffer, Engine::Handle engine);
				void drop(Texture::Handle texture, Engine::Handle engine);
				void drop(Framebuffer::Handle framebuffer, Engine::Handle engine);
				void drop(Pipeline::Handle pipeline, Engine::Handle engine);

				Resource();

				// Public
				void allocate(Engine::Handle engine);

				void drop(Engine::Handle engine);

				void submit(Reference* const referenceMemory, MemoryType memoryType, const std::span<const Buffer::Specification> specifications);
				void submit(Reference* const referenceMemory, const std::span<const Buffer::Dynamic::Specification> specifications);
				void submit(Reference* const referenceMemory, const std::span<const Texture::Specification> specifications);
				void submit(Reference* const referenceMemory, const std::span<const DescriptorSet::Specification> specifications);
				void submit(Reference* const referenceMemory, const std::span<const Pipeline::Specification> specifications);
				void submit(Reference* const referenceMemory, const std::span<const Framebuffer::Specification> specifications);
			};

			typedef Resource* Handle;

			template <Allocator::AllocatorType AllocatorType>
			Handle create(AllocatorType* allocator) {
				return Allocator::allocateResource<Resource>(allocator);
			}

			template <Allocator::AllocatorType AllocatorType>
			void drop(AllocatorType* allocator, Handle handle, Engine::Handle engine) {
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);
				VIVIUM_CHECK_HANDLE_EXISTS(handle);

				handle->drop(engine);

				Allocator::dropResource(allocator, handle);
			}

			void allocate(Engine::Handle engine, Handle handle);

			std::vector<Buffer::PromisedHandle> submit(Handle handle, MemoryType memoryType, const std::span<const Buffer::Specification> specifications);
			std::vector<Buffer::Dynamic::PromisedHandle> submit(Handle handle, const std::span<const Buffer::Dynamic::Specification> specifications);
			std::vector<Texture::PromisedHandle> submit(Handle handle, const std::span<const Texture::Specification> specifications);
			std::vector<DescriptorSet::PromisedHandle> submit(Handle handle, const std::span<const DescriptorSet::Specification> specifications);
			std::vector<Pipeline::PromisedHandle> submit(Handle handle, const std::span<const Pipeline::Specification> specifications);
			std::vector<Framebuffer::PromisedHandle> submit(Handle handle, const std::span<const Framebuffer::Specification> specifications);
		}
	}
}