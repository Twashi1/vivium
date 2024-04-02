#include "batch.h"

namespace Vivium {
	namespace Batch {
		Specification::Specification(uint64_t vertexCount, uint64_t indexCount, Buffer::Layout bufferLayout)
			: vertexCount(vertexCount), indexCount(indexCount), bufferLayout(bufferLayout)
		{}

		bool Resource::isNull() const
		{
			return vertexStaging == VK_NULL_HANDLE;
		}

		void Resource::drop(Engine::Handle engine)
		{
			Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, vertexStaging, engine);
			Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, vertexDevice, engine);
			Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, indexStaging, engine);
			Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, indexDevice, engine);
		}

		void Resource::create(Engine::Handle engine, ResourceManager::Static::Handle resourceManager, Specification specification)
		{
			std::vector<Buffer::Handle> staging = ResourceManager::Static::submit(resourceManager, MemoryType::STAGING, std::vector<Buffer::Specification>({
				Buffer::Specification(specification.vertexCount * specification.bufferLayout.stride, Buffer::Usage::STAGING),
				Buffer::Specification(specification.indexCount * sizeof(uint16_t), Buffer::Usage::STAGING)
			}));

			std::vector<Buffer::Handle> device = ResourceManager::Static::submit(resourceManager, MemoryType::DEVICE, std::vector<Buffer::Specification>({
				Buffer::Specification(specification.vertexCount * specification.bufferLayout.stride, Buffer::Usage::VERTEX),
				Buffer::Specification(specification.indexCount * sizeof(uint16_t), Buffer::Usage::INDEX)
			}));

			vertexStaging = staging[0];
			indexStaging = staging[1];

			vertexDevice = device[0];
			indexDevice = device[1];

			vertexBufferIndex = 0;
			indexBufferIndex = 0;
			verticesSubmitted = 0;

			bufferLayout = specification.bufferLayout;
		}

		void Resource::submitElement(uint64_t elementIndex, const void* data, uint64_t size, uint64_t offset)
		{
			const Buffer::Layout::Element& element = bufferLayout.elements[elementIndex];

			// Number of instances of element to be submitted
			uint64_t elementCount = size / element.size;
			// Index in vertex mapping of first element
			uint64_t firstElementIndex = vertexBufferIndex + element.offset;
			// Index in element data to copy in
			uint64_t elementDataIndex = 0;

			for (uint64_t i = 0; i < elementCount; i++) {
				// Calculate index in vertex mapping for this element
				uint64_t vertexIndex = firstElementIndex + bufferLayout.stride * i;

				// Copy data from element data to vertex mapping
				Buffer::set(vertexStaging, vertexIndex, reinterpret_cast<const uint8_t*>(data) + elementDataIndex * element.size, element.size, offset);

				++elementDataIndex;
			}
		}

		void Resource::submitRectangle(uint64_t elementIndex, float left, float bottom, float right, float top)
		{
			// TODO: make sure this works
			/*
			float data[8] = {
				left, bottom,
				right, bottom,
				right, top,
				left, top
			};

			submitElement(elementIndex, data, sizeof(data), 0);
			*/

			uint64_t vertexIndex = vertexBufferIndex + bufferLayout.elements[elementIndex].offset;

			float* dataPointer = reinterpret_cast<float*>(&reinterpret_cast<uint8_t*>(Buffer::getMapping(vertexStaging))[vertexIndex]);

			dataPointer[0] = left;
			dataPointer[1] = bottom;

			dataPointer += bufferLayout.stride / 4;

			dataPointer[0] = right;
			dataPointer[1] = bottom;

			dataPointer += bufferLayout.stride / 4;

			dataPointer[0] = right;
			dataPointer[1] = top;

			dataPointer += bufferLayout.stride / 4;

			dataPointer[0] = left;
			dataPointer[1] = top;
		}

		void Resource::endShape(uint64_t vertexCount, const uint16_t* indicies, uint64_t indicesCount, uint64_t indicesOffset)
		{
			uint16_t* indexMapping = reinterpret_cast<uint16_t*>(Buffer::getMapping(indexStaging));

			for (uint64_t i = 0; i < indicesCount; i++) {
				indexMapping[indexBufferIndex + i] = indicies[i] + verticesSubmitted;
			}

			indexBufferIndex += indicesCount;
			vertexBufferIndex += vertexCount * bufferLayout.stride;
			verticesSubmitted += vertexCount;
		}

		Result Resource::finish(Commands::Context::Handle context, Engine::Handle engine)
		{
			Result result;

			result.vertexBuffer = vertexDevice;
			result.indexBuffer = indexDevice;

			Commands::Context::beginTransfer(context);
			Commands::transferBuffer(context, vertexStaging, vertexDevice);
			Commands::transferBuffer(context, indexStaging, indexDevice);
			Commands::Context::endTransfer(context, engine);

			result.indexCount = indexBufferIndex;

			indexBufferIndex = 0;
			vertexBufferIndex = 0;
			verticesSubmitted = 0;

			return result;
		}
	}
}