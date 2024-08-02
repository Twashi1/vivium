#include "registry.h"

namespace Vivium {
	Registry::Registry()
		: signatures(Signature{})
	{
		for (uint64_t i = 0; i < componentPools.size(); i++) {
			componentPools[i] = nullptr;
		}
	}

	Registry::~Registry()
	{
		for (ComponentArray* pool : componentPools) {
			delete pool;
		}
	}

	void Registry::free(Entity entity)
	{
		for (ComponentArray* pool : componentPools) {
			if (pool == nullptr) continue;
			if (!pool->contains(entity)) continue;

			pool->free(entity);
			signatures.index(getIdentifier(entity)).set(pool->manager.componentIDFunction(), 0);
		}

		++availableEntities;
		// Build implicit list
		Entity& destroyedEntity = entities[getIdentifier(entity)];
		// Increment version number
		destroyedEntity += (1 << 20);
		std::swap(destroyedEntity, nextEntity);
	}

	void Registry::clear()
	{
		// TODO: current implementation doesn't take into account version numbers
		// Clear all component pools
		for (ComponentArray* pool : componentPools) {
			if (pool == nullptr) continue;

			pool->clear();
		}
		// Clear all entities and signatures
		availableEntities = 0;
		nextEntity = ECS_ENTITY_MAX;

		entities = {};

		signatures = { ECS_ENTITY_MAX };

		for (GroupMetadata* group : groups) {
			delete group;
		}

		groups = {};

		nextLargestEntity = 0;
	}

	Entity Registry::create()
	{
		if (availableEntities > 0) {
			Entity nextNext = entities[getIdentifier(nextEntity)];
			std::swap(nextNext, nextEntity);
			--availableEntities;

			return nextNext;
		}

		entities.push_back(nextLargestEntity);
		return nextLargestEntity++;
	}
	
	void Registry::moveEntityIntoOwningGroup(Entity entity, Signature const& signature)
	{
		std::vector<GroupMetadata*> affectedGroups;

		bool movedEntity = false;

		for (ComponentArray* pool : componentPools) {
			if (pool == nullptr) continue;
			if (!pool->isOwned()) continue;
			if (!pool->owner->containsSignature(signature)) continue;

			GroupMetadata* relevantGroup = pool->owner;

			// Not already in our affected groups
			if (std::find(affectedGroups.begin(), affectedGroups.end(), relevantGroup) == affectedGroups.end()) {
				affectedGroups.push_back(relevantGroup);
			}

			Entity const& replacementEntity = pool->entities[pool->owner->groupSize];
			pool->swap(entity, replacementEntity);

			movedEntity = true;
		}

		// Increment group sizes
		if (movedEntity) {
			for (GroupMetadata* group : affectedGroups) {
				++group->groupSize;
			}
		}
	}
}