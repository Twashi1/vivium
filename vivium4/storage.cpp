#include "storage.h"

namespace Vivium {
	namespace Allocator {
		Linear::Linear()
			: data(nullptr),
			offset(0),
			capacity(0)
		{}
		
		void Linear::reserveExact(uint64_t moreBytes) {
			uint64_t newCapacity = capacity + moreBytes;

			uint8_t* newData = reinterpret_cast<uint8_t*>(std::malloc(newCapacity));

			if (data != nullptr)
				std::memcpy(data, newData, offset);

			std::free(data);

			data = newData;
			capacity = newCapacity;
		}
		
		void Linear::reserve(uint64_t moreBytes) {
			uint64_t minimumCapacity = offset + moreBytes;

			if (minimumCapacity < capacity) return;

			uint64_t newCapacity = capacity + (capacity >> 1) + moreBytes;

			reserveExact(newCapacity - capacity);
		}
		
		void* Linear::allocate(uint64_t bytes) {
			reserve(bytes);

			uint8_t* location = data + offset;

			offset += bytes;

			return location;
		}
		
		void Linear::free() {
			std::free(data);

			data = nullptr;
			capacity = 0;
			offset = 0;
		}
	}

	namespace Storage {

	}
}