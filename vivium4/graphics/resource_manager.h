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
		struct SharedTrackerData {
			std::atomic_uint32_t deviceMemoryAllocations;

			SharedTrackerData();
		};

		inline SharedTrackerData sharedTrackerData;

		namespace Static {
			struct Resource {
				struct DeviceMemoryHandle {
					VkDeviceMemory memory;
					void* mapping;

					DeviceMemoryHandle();
				};

				std::vector<Buffer::Specification> hostBufferSpecifications;
				std::vector<Buffer::Specification> deviceBufferSpecifications;
				std::vector<Buffer::Dynamic::Specification> dynamicHostBufferSpecifications;
				std::vector<Texture::Specification> textureSpecifications;
				std::vector<Framebuffer::Specification> framebufferSpecifications;
				std::vector<DescriptorSet::Specification> descriptorSetSpecifications;
				std::vector<Pipeline::Specification> pipelineSpecifications;

				std::vector<std::span<Buffer::Resource>> hostBufferMemory;
				std::vector<std::span<Buffer::Resource>> deviceBufferMemory;
				std::vector<std::span<Buffer::Dynamic::Resource>> dynamicHostBufferMemory;
				std::vector<std::span<Texture::Resource>> textureMemory;
				std::vector<std::span<Framebuffer::Resource>> framebufferMemory;
				std::vector<std::span<DescriptorSet::Resource>> descriptorSetMemory;
				std::vector<std::span<Pipeline::Resource>> pipelineMemory;

				std::vector<DeviceMemoryHandle> deviceMemoryHandles;
				std::vector<VkDescriptorPool> descriptorPools;

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

				// Public
				void allocate(Engine::Handle engine);

				void drop(Engine::Handle engine);
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

			template <typename ResourceType>
			std::span<ResourceType> _getSpan(Handle handle, ResourceType** memory, uint64_t count)
			{
				ResourceType* resourceSpan = reinterpret_cast<ResourceType*>(handle->resourceAllocator.allocate(0, sizeof(ResourceType) * count));

				for (uint64_t i = 0; i < count; i++) {
					memory[i] = resourceSpan + i;
				}

				return { resourceSpan, count };
			}

			void submit(Handle handle, Buffer::Handle* memory, MemoryType memoryType, const std::span<const Buffer::Specification> specifications);
			void submit(Handle handle, Buffer::Dynamic::Handle* memory, const std::span<const Buffer::Dynamic::Specification> specifications);
			void submit(Handle handle, Texture::Handle* memory, const std::span<const Texture::Specification> specifications);
			void submit(Handle handle, DescriptorSet::Handle* memory, const std::span<const DescriptorSet::Specification> specifications);
			void submit(Handle handle, Pipeline::Handle* memory, const std::span<const Pipeline::Specification> specifications);
			void submit(Handle handle, Framebuffer::Handle* memory, const std::span<const Framebuffer::Specification> specifications);
		}
	}
}