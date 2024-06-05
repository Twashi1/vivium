#pragma once

// TODO: rename this file
// TODO: every resource allocated should be tracked in debug mode

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <concepts>

#include "core.h"
#include "math/math.h"

#define VIVIUM_NULL_STORAGE (reinterpret_cast<Vivium::Storage::Null*>(NULL))

namespace Vivium {
	namespace Storage {
		struct Null {};

		namespace Static {
			template <typename T>
			concept StorageType = requires(T allocator) {
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

			// Allocation of a resource in an already given memory location
			struct Inplace {
				void* location;

				Inplace() = default;
				Inplace(void* location);

				void* allocate(uint64_t);
				void free();
			};
		}

		namespace Dynamic {
			template <typename T>
			concept StorageType = requires(T allocator) {
				{ allocator.allocate(std::declval<uint64_t>()) } -> std::same_as<void*>;
				{ allocator.free() } -> std::same_as<void>;
				{ allocator.free(std::declval<void*>()) } -> std::same_as<void>;
			};
		}

		template <typename T>
		concept StorageType = Dynamic::StorageType<T> || Static::StorageType<T> || std::is_same_v<Null, T>;

		template <typename Resource, StorageType Storage>
		Resource* allocateResource(Storage* allocator) {
			VIVIUM_ASSERT(allocator != nullptr, "Can't allocate using null allocator");

			Resource* handle = reinterpret_cast<Resource*>(allocator->allocate(sizeof(Resource)));

			new (handle) Resource();

			return handle;
		}

		template <typename Resource, StorageType Storage>
		void dropResource(Storage* allocator, Resource* handle) {
			handle->~Resource();

			if constexpr (Dynamic::StorageType<Storage>)
				allocator->free(handle);
		}
	}
}