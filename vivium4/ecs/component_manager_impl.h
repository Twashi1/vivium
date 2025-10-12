#pragma once

#include "component_manager.h"

namespace Vivium {
template <ValidComponent T>
void defaultMoveComponent(void* source, void* dest) {
  // TODO: priority order...
  //	this prioritises the slower copy construction
  if constexpr (std::is_trivial_v<T> || std::is_copy_constructible_v<T>) {
    new (dest) T(*reinterpret_cast<T*>(source));
  } else if constexpr (std::is_move_constructible_v<T>) {
    new (dest) T(std::move(*reinterpret_cast<T*>(source)));
  } else {
    // TODO: assertion failed too much?
    // static_assert(false && "Failed to specialise move component");
    int x = 5;
  }
  // TODO: test this assertion fails
}

template <ValidComponent T>
void defaultCopyComponent(void const* source, void* dest) {
  new (dest) T(*reinterpret_cast<T const*>(source));
}

template <ValidComponent T>
void defaultReallocComponent(void* source, void* dest, uint64_t count) {
  for (uint64_t i = 0; i < count; i++) {
    defaultMoveComponent<T>(reinterpret_cast<T*>(source) + i,
                            reinterpret_cast<T*>(dest) + i);
  }
}

template <ValidComponent T>
void defaultDestroyComponent(void* data, uint64_t count) {
  for (uint64_t i = 0; i < count; i++) {
    (reinterpret_cast<T*>(data) + i)->~T();
  }
}

template <ValidComponent T>
void defaultSwapComponent(void* a, void* b) {
  std::swap(*reinterpret_cast<T*>(a), *reinterpret_cast<T*>(b));
}

template <typename T>
void defaultSerialiseWrite(void const* src, SerialiserMemoryInterface& store) {
  T const& value = *reinterpret_cast<T const*>(src);
  dispatchSerialiseWrite(value, store);
}

template <typename T>
void defaultSerialiseRead(void* src, SerialiserMemoryInterface& store) {
  T* value = reinterpret_cast<T*>(src);
  dispatchSerialiseRead(value, store);
}

template <typename T>
ComponentManager defaultComponentManager(uint64_t componentTypeIndex) {
  ComponentManager manager;

  manager.moveFunction = defaultMoveComponent<T>;
  manager.copyFunction = defaultCopyComponent<T>;
  manager.reallocFunction = defaultReallocComponent<T>;
  manager.destroyFunction = defaultDestroyComponent<T>;
  manager.swapFunction = defaultSwapComponent<T>;
  manager.writeFunction = defaultSerialiseWrite<T>;
  manager.readFunction = defaultSerialiseRead<T>;
  manager.componentTypeIndex = componentTypeIndex;
  manager.typeSize = sizeof(T);

  return manager;
}
}  // namespace Vivium