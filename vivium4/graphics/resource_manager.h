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
#include <unordered_set>

namespace Vivium {
	template <typename Resource>
	union Ref {};

	template <>
	union Ref<Pipeline> {
		Pipeline resource;
		PipelineReference reference;
	};

	template <>
	union Ref<Buffer> {
		Buffer resource;
		BufferReference reference;
	};

	template <>
	union Ref<Shader> {
		Shader resource;
		ShaderReference reference;
	};

	template <>
	union Ref<DescriptorLayout> {
		DescriptorLayout resource;
		DescriptorLayoutReference reference;
	};

	template <>
	union Ref<DescriptorSet> {
		DescriptorSet resource;
		DescriptorSetReference reference;
	};

	template <>
	union Ref<Framebuffer> {
		Framebuffer resource;
		FramebufferReference reference;
	};

	template <>
	union Ref<Texture> {
		Texture resource;
		TextureReference reference;
	};

	struct SharedTrackerData {
		std::atomic_uint32_t deviceMemoryAllocations;

		SharedTrackerData();
	};

	inline SharedTrackerData sharedTrackerData;

	struct ResourceManager {
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

	ResourceManager::DeviceMemoryHandle _allocateDeviceMemory(ResourceManager& manager, Engine& engine, uint32_t memoryTypeBits, MemoryType memoryType, uint64_t size);

	// 1.
	void _allocateBuffers(ResourceManager& manager, Engine& engine, MemoryType memoryType);
	void _allocateTextures(ResourceManager& manager, Engine& engine);
	void _allocateFramebuffers(ResourceManager& manager, Engine& engine);
	void _allocateDescriptorLayouts(ResourceManager& manager, Engine& engine);
	void _allocateShaders(ResourceManager& manager, Engine& engine);

	// 2.
	void _allocateDescriptorSets(ResourceManager& manager, Engine& engine);

	// 3.
	void _allocatePipelines(ResourceManager& manager, Engine& engine);

	void allocateManager(ResourceManager& manager, Engine& engine);
	void clearManagerReferences(ResourceManager& manager);

	ResourceManager createManager();
	void dropManager(ResourceManager& manager, Engine& engine);

	template <typename Reference, typename Specification, typename VResource>
	void _addToResourceField(ResourceManager::ResourceField<Specification, VResource>& resourceField, Reference* memory, std::span<Specification const> const specifications) {
		uint64_t requiredSize = specifications.size() + resourceField.specifications.size();
		uint64_t nextGrowth = (resourceField.specifications.size() >> 1) + resourceField.specifications.size();
		resourceField.specifications.reserve(std::max(nextGrowth, requiredSize));

		uint64_t currentSize = resourceField.resources.size();

		for (uint64_t i = 0; i < specifications.size(); i++) {
			// TODO: Assuming existence of a parameter, use a concept on the reference? not really necessary since we only expose this
			//	through a specialisation guarded method (i think)
			memory[i].referenceIndex = i + currentSize;
		}

		// TODO: a bit messy
		// TODO: does resize force a certain sized allocation? hopefully not?
		resourceField.resources.reserve(std::max(nextGrowth, requiredSize));
		resourceField.resources.resize(requiredSize);

		resourceField.specifications.insert(resourceField.specifications.end(), specifications.begin(), specifications.end());
	}

	void submitResource(ResourceManager& manager, BufferReference* memory, MemoryType memoryType, const std::span<const BufferSpecification> specifications);
	void submitResource(ResourceManager& manager, TextureReference* memory, const std::span<const TextureSpecification> specifications);
	void submitResource(ResourceManager& manager, FramebufferReference* memory, const std::span<const FramebufferSpecification> specifications);
	void submitResource(ResourceManager& manager, DescriptorLayoutReference* memory, const std::span<const DescriptorLayoutSpecification> specifications);
	void submitResource(ResourceManager& manager, ShaderReference* memory, const std::span<const ShaderSpecification> specifications);
	void submitResource(ResourceManager& manager, DescriptorSetReference* memory, const std::span<const DescriptorSetSpecification> specifications);
	void submitResource(ResourceManager& manager, PipelineReference* memory, const std::span<const PipelineSpecification> specifications);

	Buffer& _getReference(ResourceManager& manager, BufferReference reference);
	Texture& _getReference(ResourceManager& manager, TextureReference reference);
	Framebuffer& _getReference(ResourceManager& manager, FramebufferReference reference);
	Shader& _getReference(ResourceManager& manager, ShaderReference reference);
	DescriptorLayout& _getReference(ResourceManager& manager, DescriptorLayoutReference reference);
	DescriptorSet& _getReference(ResourceManager& manager, DescriptorSetReference reference);
	Pipeline& _getReference(ResourceManager& manager, PipelineReference reference);

	template <typename Resource>
	void convertResourceReference(ResourceManager& manager, Ref<Resource>& resourceReference)
	{
		resourceReference.resource = _getReference(manager, resourceReference.reference);
	}
}