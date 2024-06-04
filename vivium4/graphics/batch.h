#pragma once

#include "primitives/buffer.h"
#include "resource_manager.h"
#include "commands.h"

namespace Vivium {
	namespace Batch {
		struct Specification {
			uint64_t vertexCount, indexCount;

			Buffer::Layout bufferLayout;

			Specification(uint64_t vertexCount, uint64_t indexCount, Buffer::Layout bufferLayout);
			Specification() = default;
		};

		struct Resource {
			uint64_t vertexBufferIndex, indexBufferIndex, verticesSubmitted;
			uint32_t lastSubmissionIndexCount;

			Buffer::Layout bufferLayout;

			Buffer::Handle vertexStaging, indexStaging, vertexDevice, indexDevice;
		};

		typedef Resource* Handle;
		typedef Resource* PromisedHandle;

		bool isNull(const Handle handle);

		void submitElement(Handle handle, uint64_t elementIndex, const std::span<const uint8_t> data);
		void submitRectangle(Handle handle, uint64_t elementIndex, float left, float bottom, float right, float top);
		void endShape(Handle handle, uint64_t vertexCount, const std::span<const uint16_t> indices);

		Buffer::Handle vertexBuffer(Batch::Handle batch);
		Buffer::Handle indexBuffer(Batch::Handle batch);
		// Returns index count of last endSubmission
		uint32_t indexCount(Batch::Handle batch);

		void endSubmission(Handle handle, Commands::Context::Handle context, Engine::Handle engine);

		template <Allocator::AllocatorType AllocatorType>
		void drop(AllocatorType* allocator, Handle handle, Engine::Handle engine)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);
			VIVIUM_CHECK_HANDLE_EXISTS(handle);

			Buffer::drop(VIVIUM_NULL_ALLOCATOR, handle->vertexStaging, engine);
			Buffer::drop(VIVIUM_NULL_ALLOCATOR, handle->vertexDevice, engine);
			Buffer::drop(VIVIUM_NULL_ALLOCATOR, handle->indexStaging, engine);
			Buffer::drop(VIVIUM_NULL_ALLOCATOR, handle->indexDevice, engine);

			Allocator::dropResource(allocator, handle);
		}

		template <Allocator::AllocatorType AllocatorType>
		PromisedHandle submit(AllocatorType* allocator, Engine::Handle engine, ResourceManager::Static::Handle manager, Specification specification)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);

			PromisedHandle handle = Allocator::allocateResource<Resource>(allocator);

			std::array<Buffer::Handle, 2> staging;

			ResourceManager::Static::submit(manager, staging.data(), MemoryType::STAGING, std::vector<Buffer::Specification>({
				Buffer::Specification(specification.vertexCount * specification.bufferLayout.stride, Buffer::Usage::STAGING),
				Buffer::Specification(specification.indexCount * sizeof(uint16_t), Buffer::Usage::STAGING)
				}));

			std::array<Buffer::Handle, 2> device;

			ResourceManager::Static::submit(manager, device.data(), MemoryType::DEVICE, std::vector<Buffer::Specification>({
				Buffer::Specification(specification.vertexCount * specification.bufferLayout.stride, Buffer::Usage::VERTEX),
				Buffer::Specification(specification.indexCount * sizeof(uint16_t), Buffer::Usage::INDEX)
				}));

			handle->vertexStaging = staging[0];
			handle->indexStaging = staging[1];

			handle->vertexDevice = device[0];
			handle->indexDevice = device[1];

			handle->vertexBufferIndex = 0;
			handle->indexBufferIndex = 0;
			handle->verticesSubmitted = 0;
			handle->lastSubmissionIndexCount = 0;

			handle->bufferLayout = specification.bufferLayout;

			return handle;
		}
	}
}