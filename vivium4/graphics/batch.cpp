#include "batch.h"

namespace Vivium {
	BatchSpecification::BatchSpecification(uint64_t vertexCount, uint64_t indexCount, BufferLayout bufferLayout)
		: vertexCount(vertexCount), indexCount(indexCount), bufferLayout(bufferLayout)
	{}

	void submitElementBatch(Batch& batch, uint64_t elementIndex, const std::span<const uint8_t> data)
	{
		const BufferLayout::Element& element = batch.bufferLayout.elements[elementIndex];

		// Number of instances of element to be submitted
		uint64_t elementCount = data.size_bytes() / element.size;
		// Index in vertex mapping of first element
		uint64_t firstElementIndex = batch.vertexBufferIndex + element.offset;
		// Index in element data to copy in
		uint64_t elementDataIndex = 0;

		for (uint64_t i = 0; i < elementCount; i++) {
			// Calculate index in vertex mapping for this element
			uint64_t vertexIndex = firstElementIndex + batch.bufferLayout.stride * i;

			// Copy data from element data to vertex mapping
			setBuffer(batch.vertexStaging.resource, vertexIndex, data.data() + elementDataIndex * element.size, element.size);

			++elementDataIndex;
		}
	}

	void submitRectangleBatch(Batch& batch, uint64_t elementIndex, float left, float bottom, float right, float top)
	{
		float data[8] = {
			left, bottom,
			right, bottom,
			right, top,
			left, top
		};

		submitElementBatch(batch, elementIndex, std::span<uint8_t>(reinterpret_cast<uint8_t*>(data), sizeof(data)));
	}

	void endShapeBatch(Batch& batch, uint64_t vertexCount, const std::span<const uint16_t> indicies)
	{
		uint16_t* indexMapping = reinterpret_cast<uint16_t*>(getBufferMapping(batch.indexStaging.resource));

		for (uint64_t i = 0; i < indicies.size(); i++) {
			indexMapping[batch.indexBufferIndex + i] = static_cast<uint16_t>(indicies[i] + batch.verticesSubmitted);
		}

		batch.indexBufferIndex += indicies.size();
		batch.vertexBufferIndex += vertexCount * batch.bufferLayout.stride;
		batch.verticesSubmitted += vertexCount;
	}

	Buffer const& vertexBufferBatch(Batch const& batch)
	{
		return batch.vertexDevice.resource;
	}

	Buffer const& indexBufferBatch(Batch const& batch)
	{
		return batch.indexDevice.resource;
	}

	uint32_t indexCountBatch(Batch const& batch)
	{
		return batch.lastSubmissionIndexCount;
	}

	void dropBatch(Batch& batch, Engine::Handle engine)
	{
		dropBuffer(batch.vertexStaging.resource, engine);
		dropBuffer(batch.vertexDevice.resource, engine);
		dropBuffer(batch.indexStaging.resource, engine);
		dropBuffer(batch.indexDevice.resource, engine);
	}

	Batch submitBatch(Engine::Handle engine, ResourceManager& manager, BatchSpecification specification)
	{
		Batch batch;

		std::array<BufferReference, 2> staging;

		submitResource(manager, staging.data(), MemoryType::STAGING, std::vector<BufferSpecification>({
			BufferSpecification(specification.vertexCount * specification.bufferLayout.stride, BufferUsage::STAGING),
			BufferSpecification(specification.indexCount * sizeof(uint16_t), BufferUsage::STAGING)
			}));

		std::array<BufferReference, 2> device;

		submitResource(manager, device.data(), MemoryType::DEVICE, std::vector<BufferSpecification>({
			BufferSpecification(specification.vertexCount * specification.bufferLayout.stride, BufferUsage::VERTEX),
			BufferSpecification(specification.indexCount * sizeof(uint16_t), BufferUsage::INDEX)
			}));

		batch.vertexStaging.reference = staging[0];
		batch.indexStaging.reference = staging[1];

		batch.vertexDevice.reference = device[0];
		batch.indexDevice.reference = device[1];

		batch.vertexBufferIndex = 0;
		batch.indexBufferIndex = 0;
		batch.verticesSubmitted = 0;
		batch.lastSubmissionIndexCount = 0;

		batch.bufferLayout = specification.bufferLayout;

		return batch;
	}

	void endSubmissionBatch(Batch& batch, CommandContext& context, Engine::Handle engine)
	{
		contextBeginTransfer(context);
		cmdTransferBuffer(context, batch.vertexStaging.resource, batch.verticesSubmitted * batch.bufferLayout.stride, 0, batch.vertexDevice.resource);
		cmdTransferBuffer(context, batch.indexStaging.resource, batch.indexBufferIndex * sizeof(uint16_t), 0, batch.indexDevice.resource);
		contextEndTransfer(context, engine);

		batch.lastSubmissionIndexCount = batch.indexBufferIndex;
		batch.indexBufferIndex = 0;
		batch.vertexBufferIndex = 0;
		batch.verticesSubmitted = 0;
	}
		
	void setupBatch(Batch& batch, ResourceManager& manager)
	{
		convertResourceReference(manager, batch.vertexStaging);
		convertResourceReference(manager, batch.vertexDevice);
		convertResourceReference(manager, batch.indexStaging);
		convertResourceReference(manager, batch.indexDevice);
	}
}