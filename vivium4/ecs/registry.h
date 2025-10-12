#pragma once

#include <unordered_map>

#include "../error/log.h"
#include "../serialiser/serialiser.h"
#include "component_array.h"
#include "defines.h"
#include "paged_array.h"
#include "type_info.h"

namespace Vivium {
struct Registry {
  PagedArray<Signature, ECS_PAGE_SIZE, ECS_ENTITY_MAX> signatures;
  // Maps a TypeGenerator type to an index within the registry
  std::unordered_map<uint64_t, uint64_t> typeIndexMap;
  uint64_t componentPoolIndex = 0;

  // TODO: test allocating 64 components
  std::array<ComponentArray*, ECS_COMPONENT_MAX> componentPools;

  // Next to be recycled
  Entity nextEntity = ECS_ENTITY_MAX;
  // Next new available
  Entity nextLargestEntity = 0;
  uint32_t availableEntities = 0;

  // All alive/dead entities
  std::vector<Entity> entities;

  std::vector<GroupMetadata*> groups;

  Registry();
  ~Registry();

  void free(Entity entity);
  void clear();
  Entity create();

  void moveEntityIntoOwningGroup(Entity entity, Signature const& signature);

  template <ValidComponent T>
  uint64_t _getTypeIndex();

  template <ValidComponent T>
  bool _isRegistered();

  template <ValidComponent T>
  ComponentArray*& _getPoolOrCreate();

  /*! \brief Resize the pool to fit n components for potential optimisation.
   * \tparam The component pool to resize.
   * \param newCapacity The minimum size to set the pool to.
   */
  template <ValidComponent T>
  void resizePool(uint64_t newCapacity);

  /*! \brief Create a pool for the given component type.
   * \tparam The component pool to create.
   */
  template <ValidComponent T>
  void registerComponent();

  /*! \brief Adding a component to an entity by a move. */
  template <ValidComponent T>
  void addComponent(Entity entity, T&& component);

  /*! \brief Adding a component to an entity by copy. */
  template <ValidComponent T>
  void addComponent(Entity entity, T const& component);

  /*! \brief Updating a component to an entity by a move.
   *
   * Can fatally error if the entity doesn't have the component.
   */
  template <ValidComponent T>
  void updateComponent(Entity entity, T&& component);

  /*! \brief Updating a component to an entity by a copy.
   *
   * Can fatally error if the entity doesn't have the component.
   */
  template <ValidComponent T>
  void updateComponent(Entity entity, T const& component);

  /*! \brief Removing a component to an entity.
   *
   * Can fatally error if the entity doesn't have the component.
   */
  template <ValidComponent T>
  void removeComponent(Entity entity);

  /*! \brief Get a component on an entity.
   * Can fatallay error if the entity doesn't have the component.
   */
  template <ValidComponent T>
  T& getComponent(Entity entity);

  /*! \brief Returns if an entity has a component. */
  template <ValidComponent T>
  bool hasComponent(Entity entity);

  // TODO: are there potential optimisations that can be applied to non-owned
  // components, i.e. if we pass a group of non-owned components
  /*! \brief Declare a grouping between a set of annotated types.
   *
   * Requires any owned components are not owned by other groups. Owned
   * components are grouped together at the start of a pool, so that iteration
   * can occur as perfect SoA. Partially owned components are useful in relation
   * to owned components - within those owned pools, any entity which also has
   * the partially owned components will be moved to the start of the pool.
   *
   * \return A self-updating view of components that can be used to iterate
   * efficiently.
   */
  template <OwnershipTag... Components>
  View<Components...> createView();

  /*! \brief Release ownership for affected pools */
  template <OwnershipTag... Components>
  void destroyView(View<Components...> const& view);
};

template <SerialiserInterface Interface>
void serialiseWrite(Registry const& registry, Interface& interface);
template <SerialiserInterface Interface>
void serialiseRead(Registry* registry, Interface& interface);
}  // namespace Vivium
