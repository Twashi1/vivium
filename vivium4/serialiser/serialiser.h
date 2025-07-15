#pragma once

#include <cstdint>
#include <concepts>
#include <fstream>
#include <span>

#include "../error/log.h"

namespace Vivium {
	// TODO: we need start/end on interfaces
	struct SerialiserFileInterface {
		std::fstream file;

		void begin(std::string fileLocation, bool readMode);
		void writeBytes(uint64_t length, void const* data);
		void readBytes(uint64_t length, void* data);
		void end();
	};

	struct SerialiserMemoryInterface {
		void* destination;
		uint64_t offset;
		uint64_t maxSize;

		void writeBytes(uint64_t length, void const* data);
		void readBytes(uint64_t length, void* data);
	};

	template <typename T>
	concept SerialiserInterface = requires(T interface) {
		{ interface.writeBytes(std::declval<uint64_t>(), std::declval<void const*>()) } -> std::same_as<void>;
		{ interface.readBytes(std::declval<uint64_t>(), std::declval<void*>()) } ->std::same_as<void>;
	};

	template <typename T>
	concept Trivial = std::is_trivially_constructible_v<T>;

	template <SerialiserInterface Interface, Trivial T>
	void serialiseWrite(T const& data, Interface& store) { store.writeBytes(sizeof(T), &data); }
	template <SerialiserInterface Interface, Trivial T>
	void serialiseRead(T* data, Interface& store) { store.readBytes(sizeof(T), data); }

	// TODO: concept on this?
	template <SerialiserInterface Interface, typename T>
	void serialiseWrite(std::span<T> const objects, Interface& store) {
		uint64_t arraySize = objects.size();
		store.writeBytes(sizeof(uint64_t), &arraySize);

		for (uint64_t i = 0; i < objects.size(); i++) {
			store.writeBytes(sizeof(T), &objects[i]);
		}
	}

	template <SerialiserInterface Interface, typename T>
	void serialiseWrite(std::vector<T> const& objects, Interface& store) {
		uint64_t arraySize = objects.size();
		store.writeBytes(sizeof(uint64_t), &arraySize);

		for (uint64_t i = 0; i < objects.size(); i++) {
			store.writeBytes(sizeof(T), &objects[i]);
		}
	}

	template <SerialiserInterface Interface>
	void serialiseWrite(std::string const& object, Interface& store) {
		uint64_t arraySize = object.size();
		store.writeBytes(sizeof(uint64_t), &arraySize);

		for (uint64_t i = 0; i < object.size(); i++) {
			store.writeBytes(sizeof(char), &object[i]);
		}
	}

	template <SerialiserInterface Interface, typename T>
	void serialiseRead(std::vector<T>* const data, Interface& store) {
		uint64_t arraySize = 0;
		store.readBytes(sizeof(uint64_t), &arraySize);

		data->resize(arraySize);

		for (uint64_t i = 0; i < data.size(); i++) {
			store.readBytes(sizeof(T), &data[i]);
		}
	}

	template <SerialiserInterface Interface>
	void serialiseRead(std::string* const data, Interface& store) {
		uint64_t arraySize = 0;
		store.readBytes(sizeof(uint64_t), &arraySize);

		data->resize(arraySize);

		for (uint64_t i = 0; i < data.size(); i++) {
			store.readBytes(sizeof(char), &data[i]);
		}
	}
}