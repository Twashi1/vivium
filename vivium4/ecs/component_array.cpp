#include "component_array.h"

namespace Vivium {
	ComponentArray::ComponentArray()
		: sparse(ECS_ENTITY_DEAD), dense(nullptr), entities(nullptr), size(0), capacity(0)
	{}

	ComponentArray::~ComponentArray()
	{
		clear();

		delete[] dense;
		delete[] entities;
	}

	void ComponentArray::_allocateForIndex(uint64_t index) {
		if (capacity <= index) {
			resize(std::max(index + 1, capacity * 2));
		}
	}

	void ComponentArray::resize(uint64_t newCapacity) {
		if (newCapacity <= capacity) { return; }

		Entity* newEntities = new Entity[newCapacity];
		if (entities != nullptr)
		{
			std::memcpy(newEntities, entities, capacity * sizeof(Entity));

			delete[] entities;
		}

		for (uint64_t i = capacity; i < newCapacity; i++) {
			newEntities[i] = ECS_ENTITY_DEAD;
		}

		entities = newEntities;

		uint8_t* newDense = new uint8_t[newCapacity * manager.typeSize];
		if (dense != nullptr)
		{
			manager.reallocFunction(dense, newDense, size);

			delete[] dense;
		}

		dense = newDense;

		capacity = newCapacity;
	}

	bool ComponentArray::contains(Entity entity) {
		return sparse.get(getIdentifier(entity)) != ECS_ENTITY_DEAD;
	}

	void ComponentArray::swap(Entity a, Entity b) {
		uint32_t& indexA = sparse.index(getIdentifier(a));
		uint32_t& indexB = sparse.index(getIdentifier(b));

		manager.swapFunction(
			&dense[indexA * manager.typeSize],
			&dense[indexB * manager.typeSize]
		);

		std::swap(entities[indexA], entities[indexB]);
		std::swap(indexA, indexB);
	}

	void ComponentArray::free(Entity entity) {
		uint32_t& index = sparse.index(getIdentifier(entity));

		manager.destroyFunction(&dense[index * manager.typeSize], 1);

		swap(index, size - 1);
		index = ECS_ENTITY_DEAD;

		--size;
	}
	
	void ComponentArray::clear()
	{
		manager.destroyFunction(dense, size);

		size = 0;
	}
}