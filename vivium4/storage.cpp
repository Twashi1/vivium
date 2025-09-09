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

bool fits(Arena& arena, uint64_t size) {
  return arena.offset + size < arena.size;
}

std::byte* allocate(Arena& arena, uint64_t size) {
  VIVIUM_ASSERT(arena.offset + size < arena.size,
                "Allocation {} was beyond arena size {}", size, arena.size);

  std::byte* location = arena.address + arena.offset;
  arena.offset += size;

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

std::byte* allocate(BlockAllocator& block, uint64_t size) {
  Arena* bestArena = nullptr;

  for (Arena& arena : block.arenas) {
    if (!fits(arena, size)) {
      continue;
    }

    bestArena = &arena;
    break;
  }

  if (bestArena == nullptr) {
    block.arenas.push_back(createArena(std::max(block.blockSize, size)));
    bestArena = &block.arenas.back();
  }

  return allocate(*bestArena, size);
}
}  // namespace Vivium
