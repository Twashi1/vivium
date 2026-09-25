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

std::byte* allocate(ImplicitListAllocator::Group& group, uint64_t size,
                    uint64_t alignment) {
  // TODO: further assumption: size is divisible by alignment
  VIVIUM_ASSERT(size >= alignment, "Group requires lenient alignment");

  if (group.next == UINT32_MAX) {
    return allocate(group.memory, size, alignment);
  }

  uint32_t blockID = group.next >> 24;
  uint32_t blockOffset = group.next & 0xFF'FF'FF;

  Arena& arena = group.memory.arenas[blockID];
  uint32_t nextOffset = *reinterpret_cast<uint32_t*>(
      arena.address + blockOffset * group.groupSize);
  std::byte* slot = arena.address + blockOffset * group.groupSize;
  group.next = nextOffset;

  return slot;
}

bool free(ImplicitListAllocator::Group& group, std::byte* memory) {
  for (uint32_t blockID = 0; blockID < group.memory.arenas.size(); blockID++) {
    Arena& arena = group.memory.arenas[blockID];

    if (memory < arena.address || memory >= arena.address + arena.size) {
      continue;
    }

    uint32_t newBlockOffset = reinterpret_cast<uintptr_t>(memory) -
                              reinterpret_cast<uintptr_t>(arena.address);
    // TODO: get rid of division
    newBlockOffset /= group.groupSize;

    uint32_t newBlockID = blockID;

    *reinterpret_cast<uint32_t*>(*memory) = group.next;
    group.next = newBlockID << 24 | newBlockOffset;

    return true;
  }

  return false;
}

void dropGroup(ImplicitListAllocator::Group& group) {
  dropBlockAllocator(group.memory);
}
ImplicitListAllocator::Group createGroup(uint32_t groupSize) {
  ImplicitListAllocator::Group group;
  group.groupSize = groupSize;
  group.memory = createBlockAllocator(8192);
  group.next = UINT32_MAX;

  return group;
}

std::byte* allocate(ImplicitListAllocator& allocator, uint64_t size,
                    uint64_t alignment) {
  if (size <= 4) {
    return allocate(allocator.group4, size, alignment);
  }

  if (size <= 8) {
    return allocate(allocator.group8, size, alignment);
  }

  if (size <= 16) {
    return allocate(allocator.group16, size, alignment);
  }

  if (size <= 32) {
    return allocate(allocator.group32, size, alignment);
  }

  if (size <= 64) {
    return allocate(allocator.group64, size, alignment);
  }

  if (size <= 256) {
    return allocate(allocator.group256, size, alignment);
  }

  if (size <= 512) {
    return allocate(allocator.group512, size, alignment);
  }

  return static_cast<std::byte*>(malloc(size));
}

void free(ImplicitListAllocator& allocator, std::byte* memory) {
  if (free(allocator.group4, memory)) return;
  if (free(allocator.group8, memory)) return;
  if (free(allocator.group16, memory)) return;
  if (free(allocator.group32, memory)) return;
  if (free(allocator.group64, memory)) return;
  if (free(allocator.group256, memory)) return;
  if (free(allocator.group512, memory)) return;
}
void freeFast(ImplicitListAllocator& allocator, std::byte* memory,
              uint64_t knownByteSize) {
  if (knownByteSize <= 4) {
    free(allocator.group4, memory);
  } else if (knownByteSize <= 8) {
    free(allocator.group8, memory);
  } else if (knownByteSize <= 16) {
    free(allocator.group16, memory);
  } else if (knownByteSize <= 32) {
    free(allocator.group32, memory);
  } else if (knownByteSize <= 64) {
    free(allocator.group64, memory);
  } else if (knownByteSize <= 256) {
    free(allocator.group256, memory);
  } else if (knownByteSize <= 512) {
    free(allocator.group512, memory);
  } else {
    free(memory);
  }
}
void dropImplicitListAllocator(ImplicitListAllocator& allocator) {
  dropGroup(allocator.group4);
  dropGroup(allocator.group8);
  dropGroup(allocator.group16);
  dropGroup(allocator.group32);
  dropGroup(allocator.group64);
  dropGroup(allocator.group256);
  dropGroup(allocator.group512);
}
ImplicitListAllocator createImplicitListAllocator() {
  ImplicitListAllocator allocator;

  allocator.group4 = createGroup(4);
  allocator.group8 = createGroup(8);
  allocator.group16 = createGroup(16);
  allocator.group32 = createGroup(32);
  allocator.group64 = createGroup(64);
  allocator.group256 = createGroup(256);
  allocator.group512 = createGroup(512);

  return allocator;
}
}  // namespace Vivium
