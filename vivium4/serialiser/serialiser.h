#pragma once

#include <cstdint>
#include <concepts>

#include <fstream>

namespace Vivium {
	struct SerialiserFileInterface {
		std::fstream file;

		void writeBytes(uint64_t length, void const* data);
		void readBytes(uint64_t length, void* data);
	};

	struct SerialiserMemoryInterface {
		void* destination;
		uint64_t offset;

		void writeBytes(uint64_t length, void const* data);
		void readBytes(uint64_t length, void* data);
	};

	template <typename T>
	concept SerialiserInterface = requires(T interface) {
		interface.writeBytes(std::declval<uint64_t>(), std::declval<void const*>());
		interface.readBytes(std::declval<uint64_t>(), std::declval<void*>());
	};

	template <SerialiserInterface Interface>
	void serialiseWrite(uint8_t data, Interface store) { store.writeBytes(sizeof(uint8_t), &data); }
	template <SerialiserInterface Interface>
	void serialiseRead(uint8_t* data, Interface store) { store.readBytes(sizeof(uint8_t), &data); }
}