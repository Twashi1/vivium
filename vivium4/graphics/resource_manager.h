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

// TODO: submit calls should take memory to fill, instead of returning vector

namespace Vivium {
	namespace ResourceManager {
		struct SharedTrackerData {
			std::atomic_uint32_t deviceMemoryAllocations;

			SharedTrackerData();
		};

		inline SharedTrackerData sharedTrackerData;

		namespace Static {
			struct Resource {
				template <typename ResourceType, typename SpecificationType>
				struct PreallocationData {
					std::vector<std::span<ResourceType>> resources;
					std::vector<SpecificationType> specifications;

					std::vector<ResourceType*> submit(Allocator::Static::Pool* allocator, const std::span<const SpecificationType> newSpecifications)
					{
						specifications.reserve(std::max(
							(specifications.size() >> 1) + specifications.size(),
							specifications.size() + newSpecifications.size()
						));

						specifications.insert(specifications.end(), newSpecifications.begin(), newSpecifications.end());

						std::vector<ResourceType*> handles(newSpecifications.size());

						ResourceType* resourceBlock = reinterpret_cast<ResourceType*>(allocator->allocate(sizeof(ResourceType) * newSpecifications.size()));

						for (uint64_t i = 0; i < newSpecifications.size(); i++) {
							new (resourceBlock + i) ResourceType{};
							handles[i] = resourceBlock + i;
						}

						resources.push_back({ resourceBlock, newSpecifications.size() });

						return handles;
					}

					void clear() {
						resources = {};
						specifications = {};
					}
				};

				struct DeviceMemoryHandle {
					VkDeviceMemory memory;
					void* mapping;

					DeviceMemoryHandle();
				};

				PreallocationData<Buffer::Resource, Buffer::Specification> hostBuffers;
				PreallocationData<Buffer::Resource, Buffer::Specification> deviceBuffers;
				PreallocationData<Buffer::Dynamic::Resource, Buffer::Dynamic::Specification> dynamicHostBuffers;
				PreallocationData<Texture::Resource, Texture::Specification> textures;
				PreallocationData<Framebuffer::Resource, Framebuffer::Specification> framebuffers;
				PreallocationData<DescriptorSet::Resource, DescriptorSet::Specification> descriptorSets;
				PreallocationData<Pipeline::Resource, Pipeline::Specification> pipelines;

				std::vector<DeviceMemoryHandle> deviceMemoryHandles;
				VkDescriptorPool descriptorPool;

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
				void allocatePipelines(Engine::Handle engine, Window::Handle window);

				Resource();

				// Public
				void allocate(Engine::Handle engine, Window::Handle window);

				void drop(Engine::Handle engine);

				std::vector<Buffer::PromisedHandle> submit(MemoryType memoryType, const std::span<const Buffer::Specification> specifications);
				std::vector<Buffer::Dynamic::PromisedHandle> submit(const std::span<const Buffer::Dynamic::Specification> specifications);
				std::vector<Texture::PromisedHandle> submit(const std::span<const Texture::Specification> specifications);
				std::vector<DescriptorSet::PromisedHandle> submit(const std::span<const DescriptorSet::Specification> specifications);
				std::vector<Pipeline::PromisedHandle> submit(const std::span<const Pipeline::Specification> specifications);
				std::vector<Framebuffer::PromisedHandle> submit(const std::span<const Framebuffer::Specification> specifications);
			};

			typedef Resource* Handle;

			template <Allocator::AllocatorType AllocatorType>
			Handle create(AllocatorType allocator) {
				return Allocator::allocateResource<Resource>(allocator);
			}

			template <Allocator::AllocatorType AllocatorType>
			void drop(AllocatorType allocator, Handle handle, Engine::Handle engine) {
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);
				VIVIUM_CHECK_HANDLE_EXISTS(handle);

				handle->drop(engine);

				Allocator::dropResource(allocator, handle);
			}

			void allocate(Engine::Handle engine, Window::Handle window, Handle handle);

			std::vector<Buffer::PromisedHandle> submit(Handle handle, MemoryType memoryType, const std::span<const Buffer::Specification> specifications);
			std::vector<Buffer::Dynamic::PromisedHandle> submit(Handle handle, const std::span<const Buffer::Dynamic::Specification> specifications);
			std::vector<Texture::PromisedHandle> submit(Handle handle, const std::span<const Texture::Specification> specifications);
			std::vector<DescriptorSet::PromisedHandle> submit(Handle handle, const std::span<const DescriptorSet::Specification> specifications);
			std::vector<Pipeline::PromisedHandle> submit(Handle handle, const std::span<const Pipeline::Specification> specifications);
			std::vector<Framebuffer::PromisedHandle> submit(Handle handle, const std::span<const Framebuffer::Specification> specifications);
		}
	}
}