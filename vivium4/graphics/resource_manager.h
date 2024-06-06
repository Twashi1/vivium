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
	template <typename Resource>
	union Ref;

	union Ref<Pipeline> {
		Pipeline resource;
		PipelineReference reference;
	};

	union Ref<Buffer> {
		Buffer resource;
		BufferReference reference;
	};

	union Ref<Shader> {
		Shader resource;
		ShaderReference reference;
	};

	union Ref<DescriptorLayout> {
		DescriptorLayout resource;
		DescriptorLayoutReference reference;
	};

	union Ref<DescriptorSet> {
		DescriptorSet resource;
		DescriptorSetReference reference;
	};

	union Ref<Framebuffer> {
		Framebuffer resource;
		FramebufferReference reference;
	};

	union Ref<Texture> {
		Texture resource;
		TextureReference reference;
	};

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

				template <typename Specification, typename Resource>
				struct ResourceField {
					std::vector<Specification> specifications;
					std::vector<Resource> resources;
				};

				ResourceField<BufferSpecification, Buffer> hostBuffers;
				ResourceField<BufferSpecification, Buffer> deviceBuffers;
				ResourceField<TextureSpecification, Texture> textures;
				ResourceField<ShaderSpecification, Shader> shaders;
				ResourceField<FramebufferSpecification, Framebuffer> framebuffers;
				ResourceField<DescriptorLayoutSpecification, DescriptorLayout> descriptorLayouts;

				ResourceField<DescriptorSetSpecification, DescriptorSet> descriptorSets;

				ResourceField<PipelineSpecification, Pipeline> pipelines;

				std::vector<DeviceMemoryHandle> deviceMemoryHandles;
				std::vector<VkDescriptorPool> descriptorPools;
			};

			typedef Resource* Handle;

			template <Storage::StorageType StorageType>
			Handle create(StorageType* allocator) {
				return Storage::allocateResource<Resource>(allocator);
			}

			Resource::DeviceMemoryHandle _allocateDeviceMemory(Handle handle, Engine::Handle engine, uint32_t memoryTypeBits, MemoryType memoryType, uint64_t size);

			// 1.
			void _allocateBuffers(Handle handle, Engine::Handle engine, MemoryType memoryType);
			void _allocateTextures(Handle handle, Engine::Handle engine);
			void _allocateFramebuffers(Handle handle, Engine::Handle engine);
			void _allocateDescriptorLayouts(Handle handle, Engine::Handle engine);
			void _allocateShaders(Handle handle, Engine::Handle engine);

			// 2.
			void _allocateDescriptorSets(Handle handle, Engine::Handle engine);

			// 3.
			void _allocatePipelines(Handle handle, Engine::Handle engine);

			void allocate(Handle handle, Engine::Handle engine);

			template <Storage::StorageType StorageType>
			void drop(StorageType* allocator, Handle handle, Engine::Handle engine) {
				// Free vulkan memory
				for (Resource::DeviceMemoryHandle deviceMemoryHandle : handle->deviceMemoryHandles) {
					if (deviceMemoryHandle.mapping != nullptr)
						vkUnmapMemory(engine->device, deviceMemoryHandle.memory);

					vkFreeMemory(engine->device, deviceMemoryHandle.memory, nullptr);
				}

				sharedTrackerData.deviceMemoryAllocations -= static_cast<uint32_t>(deviceMemoryHandles.size());

				// Free descriptor pool
				for (VkDescriptorPool descriptorPool : descriptorPools)
					vkDestroyDescriptorPool(engine->device, descriptorPool, nullptr);

				Storage::dropResource(allocator, handle);
			}

			template <typename Reference, typename Specification, typename VResource>
			void _addToResourceField(Resource::ResourceField<Specification, VResource>& resourceField, Reference* memory, std::span<Specification const> const specifications) {
				uint64_t requiredSize = specifications.size() + resourceField.specifications.size();
				uint64_t nextGrowth = (resourceField.specifications.size() >> 1) + resourceField.specifications.size();
				resourceField.specifications.reserve(std::max(nextGrowth, requiredSize));

				uint64_t currentSize = resourceField.resources.size();

				for (uint64_t i = 0; i < specifications.size(); i++) {
					// TODO: Assuming existence of a parameter
					memory[i].referenceIndex = i + currentSize;
				}

				resourceField.resources.resize(std::max(nextGrowth, requiredSize));

				resourceField.specifications.insert(resourceField.specifications.end(), specifications.begin(), specifications.end());
			}

			void submit(Handle handle, BufferReference* memory, MemoryType memoryType, const std::span<const BufferSpecification> specifications);
			void submit(Handle handle, TextureReference* memory, const std::span<const TextureSpecification> specifications);
			void submit(Handle handle, FramebufferReference* memory, const std::span<const FramebufferSpecification> specifications);
			void submit(Handle handle, DescriptorLayoutReference* memory, const std::span<const DescriptorLayoutSpecification> specifications);
			void submit(Handle handle, ShaderReference* memory, const std::span<const ShaderSpecification> specifications);
			void submit(Handle handle, DescriptorSetReference* memory, const std::span<const DescriptorSetSpecification> specifications);
			void submit(Handle handle, PipelineReference* memory, const std::span<const PipelineSpecification> specifications);

			Buffer& getReference(Handle handle, BufferReference reference);
			Texture& getReference(Handle handle, TextureReference reference);
			Framebuffer& getReference(Handle handle, FramebufferReference reference);
			Shader& getReference(Handle handle, ShaderReference reference);
			DescriptorLayout& getReference(Handle handle, DescriptorLayoutReference reference);
			DescriptorSet& getReference(Handle handle, DescriptorSetReference reference);
			Pipeline& getPipeline(Handle handle, PipelineReference reference);

			template <typename Resource>
			void getReference(Handle handle, Ref<Resource>& resourceReference)
			{
				resourceReference.resource = getReference(handle, resourceReference.reference);
			}
		}
	}
}