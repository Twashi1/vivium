#pragma once

#include <cstdint>

// TODO: implementation

namespace Vivium {
	template <typename T>
	struct Vector {
		T* data;
		uint64_t size;
		uint64_t capacity;

		// void reallocate(uint64_t requestedCapacity);
		// void sizeToFit(uint64_t requestedSize);
		// void reserve(uint64_t moreElements);
		// void reserveExact(uint64_t moreElements);
		// void resize(uint64_t newSize);
	};
}