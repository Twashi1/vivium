#pragma once

#include "primitives/buffer.h"
#include "resource_manager.h"
#include "commands.h"

namespace Vivium {
	namespace Batch {
		struct Specification {
			uint64_t vertexCount, indexCount;

			BufferLayout bufferLayout;

			Specification(uint64_t vertexCount, uint64_t indexCount, BufferLayout bufferLayout);
			Specification() = default;
		};

		struct Resource {
			uint64_t vertexBufferIndex, indexBufferIndex, verticesSubmitted;
			uint32_t lastSubmissionIndexCount;

			BufferLayout bufferLayout;

			Ref<Buffer> vertexStaging, indexStaging, vertexDevice, indexDevice;
		};

		typedef Resource* Handle;
		typedef Resource* PromisedHandle;

		void submitElement(Handle handle, uint64_t elementIndex, const std::span<const uint8_t> data);
		void submitRectangle(Handle handle, uint64_t elementIndex, float left, float bottom, float right, float top);
		void endShape(Handle handle, uint64_t vertexCount, const std::span<const uint16_t> indices);

		Buffer const& vertexBuffer(Batch::Handle batch);
		Buffer const& indexBuffer(Batch::Handle batch);
		// Returns index count of last endSubmission
		uint32_t indexCount(Batch::Handle batch);

		void endSubmission(Handle handle, Commands::Context::Handle context, Engine::Handle engine);

		template <Storage::StorageType StorageType>
		void drop(StorageType* allocator, Handle handle, Engine::Handle engine)
		{
			dropBuffer(VIVIUM_NULL_STORAGE, handle->vertexStaging.resource, engine);
			dropBuffer(VIVIUM_NULL_STORAGE, handle->vertexDevice.resource, engine);
			dropBuffer(VIVIUM_NULL_STORAGE, handle->indexStaging.resource, engine);
			dropBuffer(VIVIUM_NULL_STORAGE, handle->indexDevice.resource, engine);

			Storage::dropResource(allocator, handle);
		}

		template <Storage::StorageType StorageType>
		PromisedHandle submit(StorageType* allocator, Engine::Handle engine, ResourceManager::Static::Handle manager, Specification specification)
		{
			PromisedHandle handle = Storage::allocateResource<Resource>(allocator);

			std::array<BufferReference, 2> staging;

			ResourceManager::Static::submit(manager, staging.data(), MemoryType::STAGING, std::vector<BufferSpecification>({
				BufferSpecification(specification.vertexCount * specification.bufferLayout.stride, BufferUsage::STAGING),
				BufferSpecification(specification.indexCount * sizeof(uint16_t), BufferUsage::STAGING)
				}));

			std::array<BufferReference, 2> device;

			ResourceManager::Static::submit(manager, device.data(), MemoryType::DEVICE, std::vector<BufferSpecification>({
				BufferSpecification(specification.vertexCount * specification.bufferLayout.stride, BufferUsage::VERTEX),
				BufferSpecification(specification.indexCount * sizeof(uint16_t), BufferUsage::INDEX)
				}));

			handle->vertexStaging.reference = staging[0];
			handle->indexStaging.reference = staging[1];

			handle->vertexDevice.reference = device[0];
			handle->indexDevice.reference = device[1];

			handle->vertexBufferIndex = 0;
			handle->indexBufferIndex = 0;
			handle->verticesSubmitted = 0;
			handle->lastSubmissionIndexCount = 0;

			handle->bufferLayout = specification.bufferLayout;

			return handle;
		}

		void setup(Handle handle, ResourceManager::Static::Handle manager);
	}
}