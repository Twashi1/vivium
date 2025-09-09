#pragma once

// TODO: rename this file
// TODO: every resource allocated should be tracked in debug mode

#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <concepts>
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

bool fits(Arena& arena, uint64_t size);
std::byte* allocate(Arena& arena, uint64_t size);

struct BlockAllocator {
  std::vector<Arena> arenas;
  uint64_t blockSize;
};

BlockAllocator createBlockAllocator(uint64_t blockSize);
void dropBlockAllocator(BlockAllocator& block);

std::byte* allocate(BlockAllocator& block, uint64_t size);
}
