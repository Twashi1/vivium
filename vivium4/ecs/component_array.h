#pragma once

#include "component_manager.h"
#include "defines.h"
#include "paged_array.h"
#include "../error/log.h"

#include <vector>

namespace Vivium {
	struct ComponentArray {
		PagedArray<Entity, ECS_PAGE_SIZE, ECS_ENTITY_MAX> sparse;
		
		// Packed
		uint8_t* dense;
		Entity* entities;

		uint64_t size;
		uint64_t capacity;

		ComponentManager manager;

		ComponentArray()
			: sparse(ECS_ENTITY_DEAD), dense(nullptr), entities(nullptr), size(0), capacity(0)
		{}

		void _allocateForIndex(uint64_t index) {
			if (capacity <= index) {
				resize(std::max(index + 1, capacity * 2));
			}
		}

		template <ValidComponent T>
		void push(Entity entity, T&& component) {
			if (sparse.get(getIdentifier(entity)) != ECS_ENTITY_DEAD) {
				VIVIUM_LOG(Log::FATAL, "Entity already had component");

				return;
			}

			uint32_t index = size;
			sparse.index(getIdentifier(entity)) = index;

			_allocateForIndex(index);

			entities[index] = entity;
			
			manager.moveFunction(&component, &dense[index * manager.typeSize]);

			++size;
		}

		template <ValidComponent T>
		T& get(Entity entity) {
			uint32_t index = sparse.index(getIdentifier(entity));

			if (index == ECS_ENTITY_DEAD) {
				VIVIUM_LOG(Log::FATAL, "Entity didn't have component");
			}

			return *reinterpret_cast<T*>(&dense[index * manager.typeSize]);
		}

		void resize(uint64_t newCapacity) {
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

		bool contains(Entity entity) {
			return sparse.get(getIdentifier(entity)) != ECS_ENTITY_DEAD;
		}

		void swap(Entity a, Entity b) {
			uint32_t& indexA = sparse.index(getIdentifier(a));
			uint32_t& indexB = sparse.index(getIdentifier(b));

			manager.swapFunction(
				&dense[indexA * manager.typeSize],
				&dense[indexB * manager.typeSize]
			);

			std::swap(entities[indexA], entities[indexB]);
			std::swap(indexA, indexB);
		}

		void free(Entity entity) {
			uint32_t& index = sparse.index(getIdentifier(entity));

			manager.destroyFunction(&dense[index * manager.typeSize], 1);

			swap(index, size - 1);
			index = ECS_ENTITY_DEAD;
			
			--size;
		}
	};
}