#pragma once

#include "component_manager_impl.h"
#include "group_impl.h"
#include "registry.h"

namespace Vivium {
template <ValidComponent T>
uint64_t Registry::_getTypeIndex() {
  uint64_t typeID = type_id<T>();

  // TODO: trashy workaround
  //	type unregistered!!!
  if (!typeIndexMap.contains(typeID)) {
    registerComponent<T>();
  }

  return typeIndexMap.at(typeID);
}

template <ValidComponent T>
bool Registry::_isRegistered() {
  uint64_t componentIndex = _getTypeIndex<T>();
  ComponentArray* arr = componentPools[componentIndex];

  if (arr == nullptr) {
    return false;
  }
  if (arr->requiresDeserialise) {
    return false;
  }

  return true;
}

template <ValidComponent T>
ComponentArray*& Registry::_getPoolOrCreate() {
  uint8_t componentID = _getTypeIndex<T>();

  ComponentArray*& arr = componentPools[componentID];

  if (!_isRegistered<T>()) {
    registerComponent<T>();
  }

  return arr;
}

template <ValidComponent T>
void Registry::resizePool(uint64_t newCapacity) {
  ComponentArray* arr = _getPoolOrCreate<T>();

  if (!_isRegistered<T>()) {
    registerComponent<T>();
  }

  arr->resize(newCapacity);
}

template <ValidComponent T>
void Registry::registerComponent() {
  // TODO: just check if registered here,
  //	and then allow re-registering
  uint64_t componentID = type_id<T>();

  // Either requires deserialisation, or already registered
  if (typeIndexMap.contains(componentID)) {
    uint64_t componentIndex = typeIndexMap[componentID];
    ComponentArray* arr = componentPools[componentIndex];

    VIVIUM_ASSERT(arr != nullptr,
                  "Registered component, but unallocated array");

    if (arr->requiresDeserialise) {
      arr->deserialise<T>(componentIndex);

      return;
    }

    // Already registered component, can just fail here
  } else {
    typeIndexMap.insert({componentID, componentPoolIndex++});

    uint64_t componentIndex = typeIndexMap[componentID];
    ComponentArray*& arr = componentPools[componentIndex];

    arr = new ComponentArray();
    arr->manager = defaultComponentManager<T>(componentIndex);
    arr->resize(0);
  }
}

template <ValidComponent T>
void Registry::addComponent(Entity entity, T&& component) {
  ComponentArray* arr = _getPoolOrCreate<T>();

  arr->push<T>(entity, std::forward<T>(component));

  Signature& signature = signatures.index(getIdentifier(entity));
  uint8_t componentID = _getTypeIndex<T>();
  signature.set(componentID);

  if (arr->owner != nullptr && arr->owner->ownsSignature(signature)) {
    moveEntityIntoOwningGroup(entity, signature);
  }
}

template <ValidComponent T>
void Registry::addComponent(Entity entity, T const& component) {
  ComponentArray* arr = _getPoolOrCreate<T>();

  arr->push<T>(entity, component);

  Signature& signature = signatures.index(getIdentifier(entity));
  uint8_t componentID = _getTypeIndex<T>();
  signature.set(componentID);

  if (arr->owner != nullptr && arr->owner->ownsSignature(signature)) {
    moveEntityIntoOwningGroup(entity, signature);
  }
}

template <ValidComponent T>
void Registry::updateComponent(Entity entity, T&& component) {
  // TODO: cleanup? undefined behaviour probably? ...
  //	might not be dropping the existing entity correctly...
  getComponent<T>(entity) = std::move(component);
}

template <ValidComponent T>
void Registry::updateComponent(Entity entity, T const& component) {
  getComponent<T>(entity) = component;
}

template <ValidComponent T>
void Registry::removeComponent(Entity entity) {
  uint8_t componentID = _getTypeIndex<T>();
  ComponentArray* arr = _getPoolOrCreate<T>();

  Signature& signature = signatures.index(getIdentifier(entity));

  GroupMetadata* relevantGroup = nullptr;

  for (ComponentArray* pool : componentPools) {
    if (pool == nullptr) continue;
    if (pool->owner == nullptr) continue;
    if (!pool->owner->ownsSignature(signature)) continue;

    relevantGroup = pool->owner;

    Entity& lastEntity = pool->entities[pool->owner->groupSize - 1];
    pool->swap(lastEntity, entity);
  }

  arr->free(entity);
  signature.set(componentID, 0);

  if (relevantGroup) relevantGroup->groupSize--;
}

template <ValidComponent T>
T& Registry::getComponent(Entity entity) {
  return _getPoolOrCreate<T>()->template get<T>(entity);
}

template <ValidComponent T>
bool Registry::hasComponent(Entity entity) {
  Signature const& signature = signatures.index(getIdentifier(entity));

  return signature.test(_getTypeIndex<T>());
}

template <OwnershipTag... Components>
View<Components...> Registry::createView() {
  // TODO: check that all components are not previously owned

  // Register all components
  (
      [this] {
        if (!this->_isRegistered<typename Components::type>()) {
          this->registerComponent<typename Components::type>();
        }
      }(),
      ...);

  GroupMetadata* metadata = new GroupMetadata;
  metadata->registry = this;
  groups.push_back(metadata);

  metadata->create<Components...>();

  ComponentArray* iteratingArray = nullptr;
  uint64_t iteratingSize = std::numeric_limits<uint64_t>::max();

  bool ownedGroup = false;

  (
      [&ownedGroup, &iteratingArray, &iteratingSize, metadata, this] {
        if constexpr (IsOwnedTag<Components>::value) {
          ownedGroup = true;

          uint32_t id = this->_getTypeIndex<typename Components::type>();
          ComponentArray* pool = this->componentPools[id];

          if (pool == nullptr) return;

          if (pool->size < iteratingSize) {
            iteratingSize = pool->size;
            iteratingArray = pool;
          }

          if (pool->isOwned()) {
            VIVIUM_LOG(LogSeverity::FATAL,
                       "Couldn't create group, already owned by group");
          }

          pool->owner = metadata;
        }
      }(),
      ...);

  if (!ownedGroup) {
    (
        [&iteratingArray, &iteratingSize, this] {
          uint32_t id = this->_getTypeIndex<typename Components::type>();
          ComponentArray* pool = this->componentPools[id];

          if (pool == nullptr) return;

          if (pool->size < iteratingSize) {
            iteratingSize = pool->size;
            iteratingArray = pool;
          }
        }(),
        ...);

    metadata->groupSize = iteratingArray->size;
  }

  for (uint64_t i = 0; i < iteratingSize; i++) {
    Entity& entity = iteratingArray->entities[i];
    Signature& signature = signatures.index(getIdentifier(entity));

    if (metadata->ownsSignature(signature)) {
      moveEntityIntoOwningGroup(entity, signature);
    }
  }

  return View<Components...>{this, iteratingArray->entities, metadata};
}

// Release ownership for affected pools
template <OwnershipTag... Components>
void Registry::destroyView(View<Components...> const& view) {
  // TODO
}

template <SerialiserInterface Interface>
void serialiseWrite(Registry const& registry, Interface& interface) {
  serialiseWrite(registry.componentPoolIndex, interface);
  serialiseWrite(registry.nextEntity, interface);
  serialiseWrite(registry.nextLargestEntity, interface);
  serialiseWrite(registry.availableEntities, interface);
  serialiseWrite(registry.entities, interface);

  uint64_t validKeyCount = 0;

  // TODO: include pools of 0 size, pools which haven't been deserialised
  for (auto const& [key, index] : registry.typeIndexMap) {
    if (registry.componentPools[index] == nullptr) continue;
    if (registry.componentPools[index]->size == 0) continue;

    validKeyCount++;
  }

  serialiseWrite(validKeyCount, interface);

  for (auto const& [key, index] : registry.typeIndexMap) {
    if (registry.componentPools[index] == nullptr) continue;
    if (registry.componentPools[index]->size == 0) continue;

    serialiseWrite(key, interface);
    serialiseWrite(index, interface);
  }

  // Write paged array
  //	we assume that the paged array will have the same page size and capacity
  // between runs 	but this could only change between builds so its very safe

  // We only want to write the enabled component arrays
  for (auto const& [key, index] : registry.typeIndexMap) {
    // Write the index of the pool that exists
    VIVIUM_ASSERT(registry.componentPools[index] != nullptr,
                  "Null component pool for registered type");

    ComponentArray* componentArray = registry.componentPools[index];
    // Don't write empty component arrays
    if (componentArray->size == 0) {
      continue;
    }

    serialiseWrite(index, interface);
    serialiseWrite(*componentArray, interface);
  }

  serialiseWrite((uint32_t)0xf5ab290d, interface);
  serialiseWrite(registry.signatures, interface);
}

template <SerialiserInterface Interface>
void serialiseRead(Registry* registry, Interface& interface) {
  serialiseRead(&registry->componentPoolIndex, interface);
  serialiseRead(&registry->nextEntity, interface);
  serialiseRead(&registry->nextLargestEntity, interface);
  serialiseRead(&registry->availableEntities, interface);
  serialiseRead(&registry->entities, interface);

  uint64_t typeIndexMapSize = 0;
  serialiseRead(&typeIndexMapSize, interface);

  for (uint64_t i = 0; i < typeIndexMapSize; i++) {
    uint64_t key, index;
    serialiseRead(&key, interface);
    serialiseRead(&index, interface);

    registry->typeIndexMap.insert({key, index});
  }

  // Write paged array
  //	we assume that the paged array will have the same page size and capacity
  // between runs 	but this could only change between builds so its very safe

  for (uint64_t i = 0; i < typeIndexMapSize; i++) {
    uint64_t componentIndex = 0;
    serialiseRead(&componentIndex, interface);

    ComponentArray*& componentArray = registry->componentPools[componentIndex];
    // TODO: generalise new component array creation for safety?
    componentArray = new ComponentArray;

    serialiseRead(componentArray, interface);
  }

  uint32_t magicSanityCheck = 0;
  serialiseRead(&magicSanityCheck, interface);
  serialiseRead(&registry->signatures, interface);

  VIVIUM_ASSERT(magicSanityCheck == (uint32_t)0xf5ab290d,
                "Something wrong with reading component arrays");
}
};  // namespace Vivium
