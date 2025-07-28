#pragma once

#include <cstdint>
#include <concepts>
#include <fstream>
#include <span>

#include "../error/log.h"
#include "../core.h"

namespace Vivium {
	// TODO: we need start/end on interfaces
	struct SerialiserFileInterface {
		std::fstream file;
		uint64_t countBytes;

		void begin(std::string fileLocation, bool readMode);
		void writeBytes(uint64_t length, void const* data);
		void readBytes(uint64_t length, void* data);
		void end();
	};

	// TODO: this interface should dynamically grow to fit
	struct SerialiserMemoryInterface {
		void* destination;
		uint64_t offset;
		uint64_t maxSize;

		void begin(uint64_t capacity);
		void writeBytes(uint64_t length, void const* data);
		void readBytes(uint64_t length, void* data);
		void end();
	};

	template <typename T>
	concept SerialiserInterface = requires(T interface) {
		{ interface.writeBytes(std::declval<uint64_t>(), std::declval<void const*>()) } -> std::same_as<void>;
		{ interface.readBytes(std::declval<uint64_t>(), std::declval<void*>()) } ->std::same_as<void>;
	};

	template <typename T>
	concept Trivial = std::is_trivial_v<T> && std::is_standard_layout_v<T> && !std::is_pointer_v<T>;

	template <typename T, typename U>
	concept IsSerialisable = Trivial<T> || (SerialiserInterface<U> && requires (T const& ref, T * ptr, U & store) {
		{ serialiseWrite(ref, store) } -> std::same_as<void>;
		{ serialiseRead(ref, store) } -> std::same_as<void>;
	});

	// TODO: disable pointer types
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
		store.writeBytes(arraySize, object.data());
	}

	template <SerialiserInterface Interface, typename T>
	void serialiseRead(std::vector<T>* const data, Interface& store) {
		uint64_t arraySize = 0;
		store.readBytes(sizeof(uint64_t), &arraySize);

		data->resize(arraySize);

		for (uint64_t i = 0; i < data->size(); i++) {
			store.readBytes(sizeof(T), &(*data)[i]);
		}
	}

	template <SerialiserInterface Interface>
	void serialiseRead(std::string* const data, Interface& store) {
		uint64_t arraySize = 0;
		store.readBytes(sizeof(uint64_t), &arraySize);

		new (data) std::string(arraySize, '\0');

		store.readBytes(arraySize, data->data());
	}

	/*
	#pragma once
#include <type_traits>
#include <concepts>

// Forward-declare the interface
class SerialiserMemoryInterface {
public:
    void writeBytes(const void* data, size_t size);
};

// Concept: trivial types
template <typename T>
concept TriviallySerializable = std::is_trivial_v<T> && std::is_standard_layout_v<T>;

// Concept: ADL-detected custom serialization
template <typename T>
concept HasUserSerialiseWrite = requires(const T& t, SerialiserMemoryInterface& s) {
    serialiseWrite(t, s); // Unqualified, enables ADL
};

// Combined concept: either trivial or custom-serializable
template <typename T>
concept Serializable = TriviallySerializable<T> || HasUserSerialiseWrite<T>;

// Default impl for trivial types
template <TriviallySerializable T>
void serialiseWriteImpl(const T& value, SerialiserMemoryInterface& store) {
    store.writeBytes(&value, sizeof(T));
}

// Forwarding call that uses ADL or fallback
template <typename T>
void dispatchSerialiseWrite(const T& value, SerialiserMemoryInterface& store) {
    if constexpr (TriviallySerializable<T>) {
        serialiseWriteImpl(value, store);  // memcpy
    } else {
        serialiseWrite(value, store);      // ADL-resolved
    }
}

// Main entry point from the library
template <Serializable T>
void defaultSerialiseWrite(const void* src, SerialiserMemoryInterface& store) {
    const T& value = *reinterpret_cast<const T*>(src);
    dispatchSerialiseWrite(value, store);
}
	*/

	/*
	// This function is found via ADL if placed in same namespace
inline void serialiseWrite(const MyVec3& v, SerialiserMemoryInterface& store) {
    dispatchSerialiseWrite(v.x, store); // Works for float via default impl
    dispatchSerialiseWrite(v.y, store);
    dispatchSerialiseWrite(v.z, store);
}
	*/

	template <typename T, SerialiserInterface Store>
	void serialiseWriteImpl(T const& value, Store& store) {
		store.writeBytes(sizeof(T), &value);
	}

	template <typename T, SerialiserInterface Store>
	void serialiseReadImpl(T* value, Store& store) {
		store.readBytes(sizeof(T), value);
	}

	template <typename T, SerialiserInterface Store>
	void dispatchSerialiseWrite(T const& value, Store& store) {
		if constexpr (Trivial<T>) {
			serialiseWriteImpl(value, store);
		}
		else {
			serialiseWrite(value, store);
		}
	}

	template <typename T, SerialiserInterface Store>
	void dispatchSerialiseRead(T* value, Store& store) {
		if constexpr (Trivial<T>) {
			serialiseReadImpl(value, store);
		}
		else {
			serialiseRead(value, store);
		}
	}
}