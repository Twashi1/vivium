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

		ComponentArray();
		~ComponentArray();

		void _allocateForIndex(uint64_t index);

		void resize(uint64_t newCapacity);
		bool contains(Entity entity);
		void swap(Entity a, Entity b);
		void free(Entity entity);
		void clear();

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
	};
}