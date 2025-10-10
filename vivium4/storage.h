#pragma once

// TODO: rename this file
// TODO: every resource allocated should be tracked in debug mode

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

#include "core.h"
#include "math/math.h"

namespace Vivium {
struct Arena {
  uint64_t size;
  uint64_t offset;
  std::byte* address;
};

/*! \brief Create a memory arena with some capacity.
 *
 * Heap allocates some memory of size at least capacity.
 *
 * \param capacity The size of the arena to create.
 * \return The memory arena.
 */
Arena createArena(uint64_t capacity);
/*! \brief Free the heap-allocated arena.
 *
 * Assumes all data allocated on the arena to be properly deconstructed.
 *
 * \param arena The arena to free.
 */
void dropArena(Arena& arena);

/*! \brief Returns if an allocation of some size will fit in the arena.
 *
 * Uses the alignment, size, and the arena capacity to calculate if the new data
 * will fit.
 *
 * \param arena The arena to allocate on.
 * \param size The size of the data to allocate.
 * \param alignment The alignment of the data to allocate.
 * \return True if the data fits, False if not.
 */
bool fits(Arena& arena, uint64_t size, uint64_t alignment);
/*! \brief Allocates some object of given size and alignment on the arena.
 *
 * Will return nullptr if failed.
 *
 * \param arena The arena to allocate on.
 * \param size The size of the data to allocate.
 * \param alignment The alignment of the data to allocate.
 * \return Returns pointer to allocation or nullpointer if failed.
 */
std::byte* allocate(Arena& arena, uint64_t size, uint64_t alignment);

struct BlockAllocator {
  std::vector<Arena> arenas;
  uint64_t blockSize;
};

/*! \brief Creates a block allocator of given block size.
 *
 * Block size determines the size of individual heap allocations. If an
 * allocation exceeds block size it will allocate a larger block for that
 * allocation alone.
 *
 * \param blockSize The size of heap allocation blocks.
 * \return The block allocator.
 */
BlockAllocator createBlockAllocator(uint64_t blockSize);
/*! \brief Frees the block allocator.
 *
 * Assumes all data allocated on the block allocator to be properly
 * deconstructed.
 *
 * \param block The block allocator to free.
 */
void dropBlockAllocator(BlockAllocator& block);

/*! \brief Allocates some object of given size and alignment on the allocator.
 *
 * Will return nullptr if failed.
 *
 * \param block The block to allocate on.
 * \param size The size of the data to allocate.
 * \param alignment The alignment of the data to allocate.
 * \return Returns pointer to allocation or nullpointer if failed.
 */
std::byte* allocate(BlockAllocator& block, uint64_t size, uint64_t alignment);
}  // namespace Vivium
