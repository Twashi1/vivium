#pragma once

#include "primitives/buffer.h"
#include "resource_manager.h"
#include "commands.h"

namespace Vivium {
	namespace Batch {
		struct Specification {
			uint64_t vertexCount, indexCount;

			Buffer::Layout bufferLayout;

			Specification() = default;
			Specification(uint64_t vertexCount, uint64_t indexCount, Buffer::Layout bufferLayout);
		};

		struct Result {
			Buffer::Handle vertexBuffer, indexBuffer;
			uint64_t indexCount;
		};

		struct Resource {
			uint64_t vertexBufferIndex, indexBufferIndex, verticesSubmitted;

			Buffer::Layout bufferLayout;

			Buffer::Handle vertexStaging, indexStaging, vertexDevice, indexDevice;

			bool isNull() const;
			void drop(Engine::Handle engine);
			void create(Engine::Handle engine, ResourceManager::Static::Handle resourceManager, Specification specification);

			void submitElement(uint64_t elementIndex, const void* data, uint64_t size, uint64_t offset);
			void submitRectangle(uint64_t elementIndex, float left, float bottom, float right, float top);
			void endShape(uint64_t vertexCount, const uint16_t* indicies, uint64_t indicesCount, uint64_t indicesOffset);

			Result finish(Commands::Context::Handle context, Engine::Handle engine);
		};

		typedef Resource* Handle;

		template <Allocator::AllocatorType AllocatorType>
		void drop(AllocatorType allocator, Handle handle, Engine::Handle engine)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine);
			VIVIUM_CHECK_HANDLE_EXISTS(handle);

			handle->drop(engine);

			Allocator::dropResource(allocator, handle);
		}

		template <Allocator::AllocatorType AllocatorType>
		Handle create(AllocatorType allocator, Specification specification, Engine::Handle engine, ResourceManager::Static::Handle manager)
		{
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine);

			Handle handle = Allocator::allocateResource<Resource>(allocator);

			handle->create(engine, manager, specification);

			return handle;
		}
	}
}