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

			// TODO: prefer drop
			// Disabled for linear allocator
			void free(void* data);
		};

		template <AllocatorType Allocator, typename Resource>
		Resource* allocateResource(Allocator allocator) {
			Resource* handle = reinterpret_cast<Resource*>(allocator.allocate(sizeof(Resource)));

			new (handle) Resource();

			return handle;
		}

		template <AllocatorType Allocator, typename Resource>
		void dropResource(Allocator allocator, Resource* handle) {
			allocator->free(reinterpret_cast<void*>(handle));
		}
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