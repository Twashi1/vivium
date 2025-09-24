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

Arena createArena(uint64_t capacity);
void dropArena(Arena& arena);

bool fits(Arena& arena, uint64_t size, uint64_t alignment);
std::byte* allocate(Arena& arena, uint64_t size, uint64_t alignment);

struct BlockAllocator {
  std::vector<Arena> arenas;
  uint64_t blockSize;
};

BlockAllocator createBlockAllocator(uint64_t blockSize);
void dropBlockAllocator(BlockAllocator& block);

std::byte* allocate(BlockAllocator& block, uint64_t size, uint64_t alignment);
}  // namespace Vivium
