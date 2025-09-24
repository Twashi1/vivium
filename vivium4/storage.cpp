#include "storage.h"

namespace Vivium {
Arena createArena(uint64_t capacity) {
  Arena arena;
  arena.size = capacity;
  arena.offset = 0;
  arena.address = static_cast<std::byte*>(malloc(capacity));

  VIVIUM_ASSERT(arena.address != nullptr, "Failed to allocate arena of size {}",
                capacity);

  return arena;
}

void dropArena(Arena& arena) { free(arena.address); }

bool fits(Arena& arena, uint64_t size, uint64_t alignment) {
  uint64_t aligned = nearestMultiple(
      reinterpret_cast<uintptr_t>(arena.address + arena.offset) + size,
      alignment);
  uint64_t newOffset =
      aligned - reinterpret_cast<uint64_t>(arena.address) + size;
  return newOffset <= size;
}

std::byte* allocate(Arena& arena, uint64_t size, uint64_t alignment) {
  uint64_t nextAddress = nearestMultiple(
      reinterpret_cast<uintptr_t>(arena.address + arena.offset), alignment);
  uint64_t newOffset =
      nextAddress - reinterpret_cast<uint64_t>(arena.address) + size;

  VIVIUM_ASSERT(newOffset < arena.size,
                "Allocation {} was beyond arena size {}", size, arena.size);

  std::byte* location = arena.address + newOffset;
  arena.offset = newOffset;

  return location;
}

BlockAllocator createBlockAllocator(uint64_t blockSize) {
  BlockAllocator allocator;
  allocator.blockSize = blockSize;

  return allocator;
}

void dropBlockAllocator(BlockAllocator& block) {
  for (Arena& arena : block.arenas) {
    dropArena(arena);
  }

  block.arenas.clear();
}

std::byte* allocate(BlockAllocator& block, uint64_t size, uint64_t alignment) {
  Arena* bestArena = nullptr;

  for (Arena& arena : block.arenas) {
    if (!fits(arena, size, alignment)) {
      continue;
    }

    bestArena = &arena;
    break;
  }

  if (bestArena == nullptr) {
    block.arenas.push_back(
        createArena(std::max(block.blockSize, size + alignment)));
    bestArena = &block.arenas.back();
  }

  return allocate(*bestArena, size, alignment);
}
}  // namespace Vivium
