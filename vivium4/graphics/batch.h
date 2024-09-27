#pragma once

#include "primitives/buffer.h"
#include "resource_manager.h"
#include "commands.h"

namespace Vivium {
	struct BatchSpecification {
		uint64_t vertexCount, indexCount;

		BufferLayout bufferLayout;

		BatchSpecification(uint64_t vertexCount, uint64_t indexCount, BufferLayout bufferLayout);
		BatchSpecification() = default;
	};

	struct Batch {
		uint64_t vertexBufferIndex, indexBufferIndex, verticesSubmitted;
		uint32_t lastSubmissionIndexCount;

		BufferLayout bufferLayout;

		Ref<Buffer> vertexStaging, indexStaging, vertexDevice, indexDevice;
	};

	void submitElementBatch(Batch& batch, uint64_t elementIndex, const std::span<const uint8_t> data);
	void submitRectangleBatch(Batch& batch, uint64_t elementIndex, float left, float bottom, float right, float top);
	void endShapeBatch(Batch& batch, uint64_t vertexCount, const std::span<const uint16_t> indices);
	void endSubmissionBatch(Batch& batch, Commands::Context::Handle context, Engine::Handle engine);

	Buffer const& vertexBufferBatch(Batch const& batch);
	Buffer const& indexBufferBatch(Batch const& batch);
	// Returns index count of last endSubmission
	uint32_t indexCountBatch(Batch const& batch);

	void dropBatch(Batch& batch, Engine::Handle engine);

	Batch submitBatch(Engine::Handle engine, ResourceManager& manager, BatchSpecification specification);
	void setupBatch(Batch& handle, ResourceManager& manager);
}