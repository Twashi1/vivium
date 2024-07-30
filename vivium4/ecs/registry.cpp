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
		// TODO: how to implement?
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
}