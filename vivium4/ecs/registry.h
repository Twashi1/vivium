#pragma once

#include "paged_array.h"
#include "defines.h"
#include "component_array.h"
#include "type_info.h"

#include "../error/log.h"
#include "../serialiser/serialiser.h"

#include <unordered_map>

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

		template <ValidComponent T>
		void resizePool(uint64_t newCapacity);

		template <ValidComponent T>
		void registerComponent();

		template <ValidComponent T>
		void addComponent(Entity entity, T&& component);

		template <ValidComponent T>
		void updateComponent(Entity entity, T&& component);

		template <ValidComponent T>
		void removeComponent(Entity entity);

		template <ValidComponent T>
		T& getComponent(Entity entity);

		template <ValidComponent T>
		bool hasComponent(Entity entity);

		template <OwnershipTag... Components>
		View<Components...> createView();

		// Release ownership for affected pools
		template <OwnershipTag... Components>
		void destroyView(View<Components...> const& view);
	};

	template <SerialiserInterface Interface>
	void serialiseWrite(Registry const& registry, Interface& interface);
	template <SerialiserInterface Interface>
	void serialiseRead(Registry* registry, Interface& interface);
}