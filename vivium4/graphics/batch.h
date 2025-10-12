#pragma once

#include "commands.h"
#include "primitives/buffer.h"
#include "resource_manager.h"

namespace Vivium {
struct BatchSpecification {
  uint64_t vertexCount, indexCount;

  BufferLayout bufferLayout;

  BatchSpecification(uint64_t vertexCount, uint64_t indexCount,
                     BufferLayout bufferLayout);
  BatchSpecification() = default;
};

struct Batch {
  uint64_t vertexBufferIndex, indexBufferIndex, verticesSubmitted;
  uint32_t lastSubmissionIndexCount;

  BufferLayout bufferLayout;

  Ref<Buffer> vertexStaging, indexStaging, vertexDevice, indexDevice;
};

/*! \brief Submit data for a single element to the batch.
 *
 * For a given element in the buffer layout, this function fills in the data for
 * that element using the contiguous data passed.
 *
 * \param batch The batch to submit to.
 * \param elementIndex The index of the element in the buffer layout.
 * \param data The data to submit for the element.
 */
void submitElementBatch(Batch& batch, uint64_t elementIndex,
                        const std::span<const uint8_t> data);
/*! \brief Submit a float rectangle to a specific element in a batch.
 *
 * \param elementIndex The index of the element in the buffer layout.
 */
void submitRectangleBatch(Batch& batch, uint64_t elementIndex, float left,
                          float bottom, float right, float top);
/*! \brief End the current shape with the given number of vertices.
 *
 * \param vertexCount The number of vertices that were submitted.
 * \param indices The indices for those vertices of the shape
 */
void endShapeBatch(Batch& batch, uint64_t vertexCount,
                   const std::span<const uint16_t> indices);
/*! \brief Finalise the batch and update the buffers with the new data.
 *
 * The batch's vertex and index buffers are now ready for rendering/transfer.
 */
void endSubmissionBatch(Batch& batch, CommandContext& context, Engine& engine);

/*! \brief Get the vertex buffer of the batch
 *
 * Requires that the batch submission was ended.
 *
 * \return A reference to the buffer.
 */
Buffer const& vertexBufferBatch(Batch const& batch);
/*! \brief Get the index buffer of the batch
 *
 * Requires that the batch submission was ended.
 *
 * \return A reference to the buffer.
 */
Buffer const& indexBufferBatch(Batch const& batch);
// Returns index count of last endSubmission
uint32_t indexCountBatch(Batch const& batch);

void dropBatch(Batch& batch, Engine& engine);

Batch submitBatch(ResourceManager& manager, BatchSpecification specification);
void setupBatch(Batch& handle, ResourceManager& manager);
}  // namespace Vivium