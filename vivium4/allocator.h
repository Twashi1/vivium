#pragma once

#include <algorithm>
#include <cstdint>

namespace Vivium {
	template <typename Resource>
	struct LinearAllocator {
		using supportsVectorFree = std::true_type;
		using supportsVectorAllocate = std::true_type;

		uint8_t* m_data;
		uint64_t m_offset;
		uint64_t m_capacity;
		uint64_t m_count;

		LinearAllocator()
			: m_data(nullptr),
			m_offset(0),
			m_capacity(capacity),
			m_count(0)
		{}

		void create(uint64_t capacity) {
			m_capacity = capacity;
			m_data = std::malloc(capacity);

			// TODO: do fatal error
			if (m_data == nullptr)
				return;
		}

		void destroy() {
			std::free(m_data);
		}

		Resource* allocate(uint64_t count) {
			// TODO: Return some error type
			if (m_offset + count * sizeof(Resource) > m_capacity)
				return nullptr;

			Resource* handle = m_data + m_offset;
			m_offset += count * sizeof(Resource);
			m_count += count;

			return handle;
		}

		void free(Resource*) {
			free(1);
		}

		void free(uint32_t count) {
			m_count -= count;

			if (m_count == 0)
				m_offset = 0;
		}
	};

	// TODO: concept for allocator
	template <typename Resource, typename AllocatorBase=LinearAllocator<Resource>>
	struct Allocator {
		AllocatorBase m_allocator;

		void create(uint64_t capacity) {
			m_allocator.create(capacity);
		}
		void destroy() {
			m_allocator.destroy();
		}

		Resource* allocate() {
			return m_allocator.allocate(1);
		}
		void free(Resource* handle) {
			return m_allocator.free(handle);
		}
	};
}