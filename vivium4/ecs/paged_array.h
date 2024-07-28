#pragma once

#include <array>
#include <cstdint>

namespace Vivium {
	template <typename T, uint64_t pageSize, uint64_t capacity>
	struct PagedArray {
		static constexpr uint64_t pageCount = (capacity - 1) / pageSize + 1;

		T defaultValue;
		std::array<T*, pageCount> pages;

		PagedArray() = default;
		PagedArray(T const& defaultValue) : defaultValue(defaultValue)
		{
			for (uint64_t i = 0; i < pageCount; i++) {
				pages[i] = nullptr;
			}
		}

		T& index(uint64_t i) {
			uint64_t pageIndex = i / pageSize;
			uint64_t indexInPage = i - pageIndex * pageSize;

			T*& page = pages[pageIndex];

			if (page == nullptr) {
				page = new T[pageSize];

				for (uint64_t j = 0; j < pageSize; j++) {
					page[j] = defaultValue;
				}
			}
			
			return page[indexInPage];
		}

		T const& get(uint64_t i) {
			uint64_t pageIndex = i / pageSize;
			uint64_t indexInPage = i - pageIndex * pageSize;

			T*& page = pages[pageIndex];

			if (page == nullptr) {
				return defaultValue;
			}

			return page[indexInPage];
		}
	};
}