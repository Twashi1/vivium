#pragma once

#include <vector>

#include "../error/log.h"
#include "component_manager.h"
#include "defines.h"
#include "group.h"
#include "paged_array.h"

namespace Vivium {
struct ComponentArray {
  PagedArray<Entity, ECS_PAGE_SIZE, ECS_ENTITY_MAX> sparse;

  // Packed
  uint8_t* dense;
  Entity* entities;

  uint64_t size;
  uint64_t capacity;

  ComponentManager manager;
  GroupMetadata* owner;

  bool requiresDeserialise;

  ComponentArray();
  ~ComponentArray();

  void _allocateForIndex(uint64_t index);

  /*!	\brief Resize the component array to the given capacity.
   *
   * Will ignore any capacity smaller than the given capacity.
   *
   * \param newCapacity The new capacity of the component array.
   */
  void resize(uint64_t newCapacity);

  /*!	\brief Returns if the given entity is in the component array.
   *
   * Runs in O(1) time.
   *
   * \param entity The entity to check for membership.
   * \return True if the entity is in the component array, false otherwise.
   */
  bool contains(Entity entity);
  /*!	\brief Swap the position of two elements in the array.
   *
   * Swaps te position of both the entities and their components.
   *
   * \param a The first entity to swap.
   * \param b The second entity to swap.
   */
  void swap(Entity a, Entity b);
  /*!	\brief Remove an entity and free up space.
   *
   * Doesn't preserve ordering as it moves entity to back of the array.
   *
   * \param entity The entity to remove.
   */
  void free(Entity entity);
  /*!	\brief Clears all entities and components.
   *
   * Doesn't reset capacity, so memory can be efficiently re-used.
   */
  void clear();

  /*!	\brief Deserialises the data in this component array.
   *
   * Requires that this component array was created by reading from a file.
   *
   * \note Uses the defaultComponentManager defined for the type, cannot use
   * custom manager.
   * \tparam T The type of component to deserialise.
   * \param componentTypeIndex The type index of the component to deserialise.
   */
  template <ValidComponent T>
  void deserialise(uint64_t componentTypeIndex) {
    // Checking for valid use
    VIVIUM_ASSERT(dense != nullptr,
                  "Can't deserialise if data hasn't been loaded");
    VIVIUM_ASSERT(requiresDeserialise,
                  "Doesn't require deserialisation, misconfiguration or loaded "
                  "improperly?");

    // sparse/entities/size already in
    manager = defaultComponentManager<T>(componentTypeIndex);
    // Create new component data array
    uint8_t* components = new uint8_t[size * manager.typeSize];
    // Begin deserialising components and writing it to the new array
    // Create a memory interface to the dense array to read from
    // TODO: bad... should be some dedicated serialiser interface for memory you
    //	already own?
    SerialiserMemoryInterface store;
    store.destination = dense;
    store.offset = 0;
    store.maxSize = capacity;

    for (uint64_t i = 0; i < size; i++) {
      manager.readFunction(components + manager.getOffset(i), store);
    }

    // Replace the dense array with our own now
    delete[] dense;
    dense = components;
    capacity = size * manager.typeSize;
    requiresDeserialise = false;
  }

  /*! \brief Returns if this component array is owned by a group.
   * \return Returns true if this component array is owned.
   */
  bool isOwned() const;

  /*!	\brief Add component to entity.
   *
   * Moves the component to the entity, thus invalidating the argument.
   *
   * \param entity The entity to add the component to.
   * \param component The component to add by move.
   * \tparam T The type of component to add to this entity.
   */
  template <ValidComponent T>
  void push(Entity entity, T&& component) {
    if (sparse.get(getIdentifier(entity)) != ECS_ENTITY_DEAD) {
      VIVIUM_LOG(LogSeverity::FATAL, "Entity already had component");

      return;
    }

    uint32_t index = size;
    sparse.index(getIdentifier(entity)) = index;

    _allocateForIndex(index);

    entities[index] = entity;

    manager.moveFunction(&component, &dense[index * manager.typeSize]);

    ++size;
  }

  /*!	\brief Add component to entity.
   *
   * Copies the component to the entity, thus leaving the argument unchanged.
   *
   * \param entity The entity to add the component to.
   * \param component The component to add by copy.
   * \tparam T The type of component to add to this entity.
   */
  template <ValidComponent T>
  void push(Entity entity, T const& component) {
    if (sparse.get(getIdentifier(entity)) != ECS_ENTITY_DEAD) {
      VIVIUM_LOG(LogSeverity::FATAL, "Entity already had component");

      return;
    }

    uint32_t index = size;
    sparse.index(getIdentifier(entity)) = index;

    _allocateForIndex(index);

    entities[index] = entity;

    manager.copyFunction(&component, &dense[index * manager.typeSize]);

    ++size;
  }

  /*!	\brief Get reference to component.
   *
   * Get a reference to the component that belongs to the given entity. The
   * reference lasts until any write is made to the component array.
   *
   * \param entity The entity to add the component to.
   * \tparam T The type of component to get from this entity.
   * \return A (temporary) reference to the component.
   */
  template <ValidComponent T>
  T& get(Entity entity) {
    uint32_t index = sparse.index(getIdentifier(entity));

    if (index == ECS_ENTITY_DEAD) {
      VIVIUM_LOG(LogSeverity::FATAL, "Entity didn't have component");
    }

    return *reinterpret_cast<T*>(&dense[index * manager.typeSize]);
  }

  template <ValidComponent T>
  T& _getIndex(uint64_t index) {
    return *reinterpret_cast<T*>(&dense[index * manager.typeSize]);
  }

  template <ValidComponent T>
  T const& _getIndex(uint64_t index) const {
    return *reinterpret_cast<T const*>(&dense[index * manager.typeSize]);
  }
};

/*!	\brief Serialise any component array.
 *
 * Uses the passed writeFunction to the manager to perform serialisation.
 *
 * \param componentArray The component array to serialise.
 * \param interface The serialiser interface to write to.
 * \tparam Interface The type of serialiser interface.
 */
template <SerialiserInterface Interface>
void serialiseWrite(ComponentArray const& componentArray,
                    Interface& interface) {
  serialiseWrite(componentArray.sparse, interface);
  serialiseWrite(componentArray.size, interface);
  // TODO: best we can do?...
  interface.writeBytes(componentArray.size * sizeof(Entity),
                       componentArray.entities);

  SerialiserMemoryInterface memoryInterface;
  memoryInterface.begin(componentArray.size * componentArray.manager.typeSize);

  // Regular read in
  if (!componentArray.requiresDeserialise) {
    for (uint64_t i = 0; i < componentArray.size; i++) {
      void* componentData =
          componentArray.dense + componentArray.manager.getOffset(i);
      componentArray.manager.writeFunction(componentData, memoryInterface);
    }
  }
  // We never deserialised this component array
  //	so just write it back in as-is
  else {
    // Capacity stores number of bytes
    memoryInterface.writeBytes(componentArray.capacity, componentArray.dense);
  }

  // Write memory interface to regular interface
  serialiseWrite(memoryInterface.offset, interface);
  interface.writeBytes(memoryInterface.offset, memoryInterface.destination);

  memoryInterface.end();

  // Not serialising the component manager or the group metadata
}

/*!	\brief Deserialise any component array.
 *
 * This doesn't perform full deserialisation due to type erasure. In order to
 * fully deserialise the array you must call deserialise to pass in relevant
 * type information.
 *
 * \param componentArray Pointer to the component array to deserialise.
 * \param interface The serialiser interface to write to.
 * \tparam Interface The type of serialiser interface.
 */
template <SerialiserInterface Interface>
void serialiseRead(ComponentArray* componentArray, Interface& interface) {
  dispatchSerialiseRead(&componentArray->sparse, interface);
  dispatchSerialiseRead(&(componentArray->size), interface);
  componentArray->entities = new Entity[componentArray->size];
  interface.readBytes(componentArray->size * sizeof(Entity),
                      componentArray->entities);

  // We allocate enough space to store n elements, although this
  //	might not necessarily accommodate all the deserialised data
  //	its a good starting point

  // Number of bytes of all the component data
  uint64_t totalSize = 0;
  serialiseRead(&totalSize, interface);
  // We're gonna hijack the component array in a couple ways
  //	reusing some memory... very bad, use unions
  componentArray->dense = new uint8_t[totalSize];
  interface.readBytes(totalSize, componentArray->dense);
  // So we know the size of the array for future reference
  componentArray->capacity = totalSize;
  componentArray->manager = ComponentManager();

  componentArray->requiresDeserialise = true;
}
}  // namespace Vivium