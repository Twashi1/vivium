#pragma once

#include <concepts>
#include <cstdint>
#include <fstream>
#include <span>

#include "../core.h"
#include "../error/log.h"

namespace Vivium {
// TODO: we need start/end on interfaces
struct SerialiserFileInterface {
  std::fstream file;
  uint64_t countBytes;

  /*! \brief Open file at location for reading or writing.
   *
   * Always opens in binary mode. Non-fatal error if file is not opened.
   *
   * \param fileLocation The file location to open.
   * \param readMode True for reading, False for writing.
   */
  void begin(std::string fileLocation, bool readMode);
  /*! \brief Write some number of bytes from pointer to file.
   *
   * \param length The number of bytes to read from the pointer.
   * \param data The pointer to read from.
   */
  void writeBytes(uint64_t length, void const* data);
  /*! \brief Read some number of bytes from the file to the pointer.
   *
   * \param length The number of bytes to write to the pointer.
   * \param data The pointer to write to.
   */
  void readBytes(uint64_t length, void* data);
  /*! \brief Closes the file and saves. */
  void end();
};

// TODO: this interface should dynamically grow to fit
struct SerialiserMemoryInterface {
  void* destination;
  uint64_t offset;
  uint64_t maxSize;

  /*! \brief Create a memory arena of some number of bytes.
   *
   * \param capacity The size of the memory arena.
   */
  void begin(uint64_t capacity);
  /*! \brief The number of bytes and data from which to write to the arena.
   *
   * Reads length bytes from data, and copies those bytes to the arena.
   *
   * \param length The number of bytes to read from data.
   * \param data The data to read from.
   */
  void writeBytes(uint64_t length, void const* data);
  /*! \brief Read some number of bytes from the arena to the pointer.
   *
   * \param length The number of bytes to read from the pointer.
   * \param data The pointer to write to.
   */
  void readBytes(uint64_t length, void* data);
  /*! \brief Free the memory arena. */
  void end();
};

template <typename T>
concept SerialiserInterface = requires(T interface) {
  {
    interface.writeBytes(std::declval<uint64_t>(), std::declval<void const*>())
  } -> std::same_as<void>;
  {
    interface.readBytes(std::declval<uint64_t>(), std::declval<void*>())
  } -> std::same_as<void>;
};

template <typename T>
concept Trivial = std::is_trivial_v<T> && std::is_standard_layout_v<T> &&
                  !std::is_pointer_v<T>;

template <typename T, typename U>
concept IsSerialisable =
    Trivial<T> ||
    (SerialiserInterface<U> && requires(T const& ref, T* ptr, U& store) {
      { serialiseWrite(ref, store) } -> std::same_as<void>;
      { serialiseRead(ref, store) } -> std::same_as<void>;
    });

/*! \brief Write some trivial data to a data store.
 *
 * Disabled for pointer types.
 *
 * \param data The data to write.
 * \param store The interface to write to.
 */
template <SerialiserInterface Interface, Trivial T>
void serialiseWrite(T const& data, Interface& store) {
  store.writeBytes(sizeof(T), &data);
}
/*! \brief Read some trivial data from a data store.
 *
 * \param data The pointer to write data to.
 * \param store The interface to read from.
 */
template <SerialiserInterface Interface, Trivial T>
void serialiseRead(T* data, Interface& store) {
  store.readBytes(sizeof(T), data);
}

// TODO: concept on this?
/*! \brief Serialise some iterable span of memory.
 *
 * TODO: while this uses the span type, it doesn't exploit contiguous memory.
 *
 * \param objects The objects to write to the store.
 * \param store The interface to write to.
 */
template <SerialiserInterface Interface, typename T>
void serialiseWrite(std::span<T> const objects, Interface& store) {
  uint64_t arraySize = objects.size();
  store.writeBytes(sizeof(uint64_t), &arraySize);

  for (uint64_t i = 0; i < objects.size(); i++) {
    store.writeBytes(sizeof(T), &objects[i]);
  }
}

/*! \brief Write a vector of objects.
 *
 * \param objects The objects to write to the store.
 * \param store The interface to write to.
 */
template <SerialiserInterface Interface, typename T>
void serialiseWrite(std::vector<T> const& objects, Interface& store) {
  uint64_t arraySize = objects.size();
  store.writeBytes(sizeof(uint64_t), &arraySize);

  for (uint64_t i = 0; i < objects.size(); i++) {
    store.writeBytes(sizeof(T), &objects[i]);
  }
}

/*! \brief Write a string to the store.
 *
 * \param object The string to write.
 * \param store The interface to write to.
 */
template <SerialiserInterface Interface>
void serialiseWrite(std::string const& object, Interface& store) {
  uint64_t arraySize = object.size();
  store.writeBytes(sizeof(uint64_t), &arraySize);
  store.writeBytes(arraySize, object.data());
}

/*! \brief Read a vector of data from the store.
 *
 * Requires a pointer to an unconstructed vector.
 * TODO: requiring unintialised memory is weird convention, very dangerous.
 *
 * \param data Pointer to uninitialised memory.
 * \param store The store to read from.
 */
template <SerialiserInterface Interface, typename T>
void serialiseRead(std::vector<T>* const data, Interface& store) {
  uint64_t arraySize = 0;
  store.readBytes(sizeof(uint64_t), &arraySize);

  // Placement new a std::vector... might cause issues tho
  //	as other functions don't assume placement new
  new (data) std::vector<T>();
  data->resize(arraySize);

  for (uint64_t i = 0; i < data->size(); i++) {
    store.readBytes(sizeof(T), &(*data)[i]);
  }
}

/*! \brief Read a string from the store.
 *
 * Requires a pointer to an unconstructed string.
 * TODO: requiring unintialised memory is weird convention, very dangerous.
 *
 * \param data Pointer to uninitialised memory.
 * \param store The store to read from.
 */
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
concept TriviallySerializable = std::is_trivial_v<T> &&
std::is_standard_layout_v<T>;

// Concept: ADL-detected custom serialization
template <typename T>
concept HasUserSerialiseWrite = requires(const T& t, SerialiserMemoryInterface&
s) { serialiseWrite(t, s); // Unqualified, enables ADL
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

// TODO: these functions are not public, should reflect that
template <typename T, SerialiserInterface Store>
void serialiseWriteImpl(T const& value, Store& store) {
  store.writeBytes(sizeof(T), &value);
}

template <typename T, SerialiserInterface Store>
void serialiseReadImpl(T* value, Store& store) {
  store.readBytes(sizeof(T), value);
}

/*! \brief Perform a write operation, generalised for ADL.
 *
 * TODO: a better description of the weird functions to get this templated mess
 * working.
 *
 * \param value The value to write to store.
 * \param store The store to write to.
 */
template <typename T, SerialiserInterface Store>
void dispatchSerialiseWrite(T const& value, Store& store) {
  if constexpr (Trivial<T>) {
    serialiseWriteImpl(value, store);
  } else {
    serialiseWrite(value, store);
  }
}

/*! \brief Perform a read operation, generalised for ADL.
 *
 * TODO: a better description of the weird functions to get this templated mess
 * working.
 *
 * \param value The value to write to.
 * \param store The store to read from.
 */
template <typename T, SerialiserInterface Store>
void dispatchSerialiseRead(T* value, Store& store) {
  if constexpr (Trivial<T>) {
    serialiseReadImpl(value, store);
  } else {
    serialiseRead(value, store);
  }
}
}  // namespace Vivium
