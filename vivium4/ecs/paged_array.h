#pragma once

#include <array>
#include <cstdint>

#include "../serialiser/serialiser.h"

namespace Vivium {
	template <typename T, uint64_t pageSize, uint64_t capacity>
	struct PagedArray {
		static constexpr uint64_t pageCount = (capacity - 1) / pageSize + 1;

		T defaultValue;
		std::array<T*, pageCount> pages;

		PagedArray()
		{
			for (uint64_t i = 0; i < pageCount; i++) {
				pages[i] = nullptr;
			}
		}

		PagedArray(T const& defaultValue) : PagedArray() { this->defaultValue = defaultValue; }
		
		~PagedArray() {
			for (T* page : pages) {
				delete page;
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

	template <SerialiserInterface Interface, typename T, uint64_t pageSize, uint64_t capacity>
	void serialiseWrite(PagedArray<T, pageSize, capacity> const& pagedArray, Interface& interface) {
		// Not easily parallelised but whatever
		serialiseWrite(pagedArray.defaultValue, interface);

		for (uint64_t i = 0; i < pagedArray.pageCount; i++) {
			serialiseWrite(pagedArray.pages[i] != nullptr, interface);

			if (pagedArray.pages[i] != nullptr) {
				for (uint64_t j = 0; j < pageSize; j++) {
					serialiseWrite(pagedArray.pages[i][j], interface);
				}
			}
		}
	}

	template <SerialiserInterface Interface, typename T, uint64_t pageSize, uint64_t capacity>
	void serialiseRead(PagedArray<T, pageSize, capacity>* pagedArray, Interface& interface) {
		serialiseRead(&pagedArray->defaultValue, interface);

		for (uint64_t i = 0; i < pagedArray->pageCount; i++) {
			bool hasData = false;
			serialiseRead(&hasData, interface);

			if (hasData) {
				for (uint64_t j = 0; j < pageSize; j++) {
					serialiseRead(&pagedArray->index(i * pageSize + j), interface);
				}
			}
		}
	}
}