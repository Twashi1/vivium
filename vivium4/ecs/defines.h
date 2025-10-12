#pragma once

#include <bitset>
#include <cstdint>

#include "../serialiser/serialiser.h"

namespace Vivium {
constexpr uint64_t ECS_PAGE_SIZE = 4096U;
constexpr uint64_t ECS_MAX_TYPES = 0xffffU;

constexpr uint32_t ECS_ENTITY_MAX = 0xfffff;
constexpr uint32_t ECS_VERSION_MAX = 0xfff;
constexpr uint32_t ECS_ENTITY_DEAD = 0xffffffff;

constexpr uint32_t ECS_VERSION_MASK = 0xfff00000;
constexpr uint32_t ECS_VERSION_SHIFT = 20;
constexpr uint32_t ECS_ENTITY_MASK = 0x000fffff;

constexpr uint8_t ECS_COMPONENT_MAX = 0xff;

typedef uint32_t Entity;
typedef std::bitset<ECS_COMPONENT_MAX> Signature;

/*! \brief Get the version number of the entity as an int*/
uint32_t getVersion(Entity entity);
/*! \brief Get the identifying number of the entity as an int*/
uint32_t getIdentifier(Entity entity);

// TODO: use both
constexpr Entity nullEntity = ECS_ENTITY_MAX & ECS_ENTITY_MASK;
constexpr Entity tombstoneEntity = ECS_ENTITY_MAX & ECS_VERSION_MASK;

// TODO: test both these functions
/*! \brief Write a signature to a store. */
template <SerialiserInterface Store>
void serialiseWrite(Signature const& signature, Store& store) {
  // Get number of bytes to represent bitset
  uint8_t* bits = new uint8_t[sizeof(Signature)];
  memset(bits, 0, sizeof(Signature));

  // TODO: better solution would necessitate custom bitset type
  for (uint64_t i = 0; i < signature.size(); i++) {
    bits[i / 8] |= (signature[i] << (i % 8));
  }

  store.writeBytes(sizeof(Signature), bits);

  delete[] bits;
}

/*! \brief Read a signature to from a store. */
template <SerialiserInterface Store>
void serialiseRead(Signature* signature, Store& store) {
  uint8_t* bits = new uint8_t[sizeof(Signature)];

  store.readBytes(sizeof(Signature), bits);

  // Not necessary, but sanity
  signature->reset();

  for (uint64_t i = 0; i < sizeof(Signature); i++) {
    (*signature) |= Signature(bits[i]) << (i * 8);
  }

  delete[] bits;
}
}  // namespace Vivium
