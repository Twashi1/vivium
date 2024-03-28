#pragma once

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <concepts>

#include "core.h"

namespace Vivium {
	namespace Allocator {
		template <typename T>
		concept AllocatorType = requires(T allocator) {
			{ allocator.allocate(std::declval<uint64_t>()) } -> std::same_as<void*>;
			{ allocator.free() } -> std::same_as<void>;
			{ allocator.free(std::declval<void*>()) } -> std::same_as<void>;
		};

		struct Linear {
			uint8_t* data;
			uint64_t offset;
			uint64_t capacity;

			Linear();

			void reserveExact(uint64_t moreBytes);
			void reserve(uint64_t moreBytes);
			void* allocate(uint64_t bytes);
			void free();

			// Disabled for linear allocator
			void free(void* data);
		};
	}

	namespace Storage {
		enum Type {
			STAGING,
			DEVICE
		};

		struct Static {
			Type type;
			// TODO
		};
	}
}