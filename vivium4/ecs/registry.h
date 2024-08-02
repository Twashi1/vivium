#pragma once

#include "paged_array.h"
#include "defines.h"
#include "component_array.h"

#include "../error/log.h"

namespace Vivium {
	struct Registry {
		PagedArray<Signature, ECS_PAGE_SIZE, ECS_ENTITY_MAX> signatures;
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
		ComponentArray*& _getPoolOrCreate() {
			uint8_t componentID = TypeGenerator::getIdentifier<T>();
			
			ComponentArray*& arr = componentPools[componentID];
			
			if (arr == nullptr) { registerComponent<T>(); }

			return arr;
		}

		template <ValidComponent T>
		void resizePool(uint64_t newCapacity) {
			ComponentArray* arr = _getPoolOrCreate<T>();

			if (arr == nullptr) {
				registerComponent<T>();
			}

			arr->resize(newCapacity);
		}

		template <ValidComponent T>
		void registerComponent() {
			uint8_t componentID = TypeGenerator::getIdentifier<T>();

			ComponentArray*& arr = componentPools[componentID];

			if (arr != nullptr) {
				VIVIUM_LOG(Log::FATAL, "Already registered component");

				return;
			}

			arr = new ComponentArray();
			arr->manager = defaultComponentManager<T>();
			arr->resize(0);
		}

		template <ValidComponent T>
		void addComponent(Entity entity, T&& component) {
			uint8_t componentID = TypeGenerator::getIdentifier<T>();
			ComponentArray* arr = _getPoolOrCreate<T>();

			arr->push<T>(entity, std::forward<T>(component));

			Signature& signature = signatures.index(getIdentifier(entity));
			signature.set(componentID);

			// TODO: perform group operations
		}

		template <ValidComponent T>
		void removeComponent(Entity entity) {
			uint8_t componentID = TypeGenerator::getIdentifier<T>();
			ComponentArray* arr = _getPoolOrCreate<T>();

			// TODO: perform group operations

			arr->free(entity);

			Signature& signature = signatures.index(getIdentifier(entity));
			signature.set(componentID, 0);
		}

		template <ValidComponent T>
		T& getComponent(Entity entity) {
			return _getPoolOrCreate<T>()->get<T>(entity);
		}
	};
}