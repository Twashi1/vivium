#pragma once

// TODO: rename this file
// TODO: every resource allocated should be tracked in debug mode

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <concepts>

#include "core.h"

#define VIVIUM_RESOURCE_ALLOCATED (reinterpret_cast<Vivium::Allocator::Null*>(NULL))

namespace Vivium {
	namespace Allocator {
		struct Null {};

		namespace Static {
			template <typename T>
			concept AllocatorType = requires(T allocator) {
				{ allocator.allocate(std::declval<uint64_t>()) } -> std::same_as<void*>;
				{ allocator.free() } -> std::same_as<void>;
			};

			struct Pool {
				struct Block {
					uint8_t* data;
					uint64_t offset;

					Block() = default;
					Block(uint8_t* data, uint64_t offset);
				};

				std::vector<Block> blocks;
				uint64_t blockCapacity;

				Pool();
				Pool(uint64_t blockCapacity);

				// Public
				void* allocate(uint64_t bytes);
				void free();
			};

			struct Transient {
				uint8_t* data;
				uint64_t offset;
				
				Transient() = default;
				Transient(uint64_t totalCapacity);

				void* allocate(uint64_t bytes);
				void free();
			};
		}

		namespace Dynamic {
			template <typename T>
			concept AllocatorType = requires(T allocator) {
				{ allocator.allocate(std::declval<uint64_t>()) } -> std::same_as<void*>;
				{ allocator.free() } -> std::same_as<void>;
				{ allocator.free(std::declval<void*>()) } -> std::same_as<void>;
			};

			// TODO: tree/list allocator
		}

		template <typename T>
		concept AllocatorType = Dynamic::AllocatorType<T> || Static::AllocatorType<T> || std::is_same_v<Null, T>;

		template <typename Resource, AllocatorType Allocator>
		Resource* allocateResource(Allocator* allocator) {
			VIVIUM_ASSERT(allocator != nullptr, "Can't allocate using null allocator");

			Resource* handle = reinterpret_cast<Resource*>(allocator->allocate(sizeof(Resource)));

			new (handle) Resource();

			return handle;
		}

		template <typename Resource, AllocatorType Allocator>
		void dropResource(Allocator* allocator, Resource* handle) {
			handle->~Resource();

			if constexpr (Dynamic::AllocatorType<Allocator>)
				allocator->free(handle);
		}
	}
}