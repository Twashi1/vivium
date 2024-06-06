#include "batch.h"

namespace Vivium {
	namespace Batch {
		Specification::Specification(uint64_t vertexCount, uint64_t indexCount, BufferLayout bufferLayout)
			: vertexCount(vertexCount), indexCount(indexCount), bufferLayout(bufferLayout)
		{}

		void submitElement(Handle handle, uint64_t elementIndex, const std::span<const uint8_t> data)
		{
			const BufferLayout::Element& element = handle->bufferLayout.elements[elementIndex];

			// Number of instances of element to be submitted
			uint64_t elementCount = data.size_bytes() / element.size;
			// Index in vertex mapping of first element
			uint64_t firstElementIndex = handle->vertexBufferIndex + element.offset;
			// Index in element data to copy in
			uint64_t elementDataIndex = 0;

			for (uint64_t i = 0; i < elementCount; i++) {
				// Calculate index in vertex mapping for this element
				uint64_t vertexIndex = firstElementIndex + handle->bufferLayout.stride * i;

				// Copy data from element data to vertex mapping
				setBuffer(handle->vertexStaging.resource, vertexIndex, data.data() + elementDataIndex * element.size, element.size);

				++elementDataIndex;
			}
		}

		void submitRectangle(Handle handle, uint64_t elementIndex, float left, float bottom, float right, float top)
		{
			float data[8] = {
				left, bottom,
				right, bottom,
				right, top,
				left, top
			};

			submitElement(handle, elementIndex, std::span<uint8_t>(reinterpret_cast<uint8_t*>(data), sizeof(data)));
		}

		void endShape(Handle handle, uint64_t vertexCount, const std::span<const uint16_t> indicies)
		{
			uint16_t* indexMapping = reinterpret_cast<uint16_t*>(getBufferMapping(handle->indexStaging.resource));

			for (uint64_t i = 0; i < indicies.size(); i++) {
				indexMapping[handle->indexBufferIndex + i] = static_cast<uint16_t>(indicies[i] + handle->verticesSubmitted);
			}

			handle->indexBufferIndex += indicies.size();
			handle->vertexBufferIndex += vertexCount * handle->bufferLayout.stride;
			handle->verticesSubmitted += vertexCount;
		}

		Buffer const& vertexBuffer(Batch::Handle batch)
		{
			return batch->vertexDevice.resource;
		}

		Buffer const& indexBuffer(Batch::Handle batch)
		{
			return batch->indexDevice.resource;
		}

		uint32_t indexCount(Batch::Handle batch)
		{
			return batch->lastSubmissionIndexCount;
		}

		void endSubmission(Handle handle, Commands::Context::Handle context, Engine::Handle engine)
		{
			Commands::Context::beginTransfer(context);
			Commands::transferBuffer(context, handle->vertexStaging, handle->vertexDevice);
			Commands::transferBuffer(context, handle->indexStaging, handle->indexDevice);
			Commands::Context::endTransfer(context, engine);

			handle->lastSubmissionIndexCount = handle->indexBufferIndex;
			handle->indexBufferIndex = 0;
			handle->vertexBufferIndex = 0;
			handle->verticesSubmitted = 0;
		}
		
		void setup(Handle handle, ResourceManager::Static::Handle manager)
		{
			ResourceManager::Static::getReference(manager, handle->vertexStaging);
			ResourceManager::Static::getReference(manager, handle->vertexDevice);
			ResourceManager::Static::getReference(manager, handle->indexStaging);
			ResourceManager::Static::getReference(manager, handle->indexDevice);
		}
	}
}