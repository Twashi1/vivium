#pragma once

#include "registry.h"
#include "group_impl.h"
#include "component_manager_impl.h"

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

		if (arr == nullptr) { return false; }
		if (arr->requiresDeserialise) { return false; }

		return true;
	}

	template <ValidComponent T>
	ComponentArray*& Registry::_getPoolOrCreate() {
		uint8_t componentID = _getTypeIndex<T>();

		ComponentArray*& arr = componentPools[componentID];

		if (!_isRegistered<T>()) { registerComponent<T>(); }

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
		typeIndexMap.insert({ componentID, componentPoolIndex++ });

		uint64_t componentIndex = componentPoolIndex - 1;

		ComponentArray*& arr = componentPools[componentIndex];

		if (arr != nullptr) {
			if (arr->requiresDeserialise) {
				arr->deserialise<T>(componentIndex);
			}
			else {
				VIVIUM_LOG(LogSeverity::FATAL, "Already registered component");
			}

			return;
		}

		arr = new ComponentArray();
		arr->manager = defaultComponentManager<T>(componentIndex);
		arr->resize(0);
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
	void Registry::updateComponent(Entity entity, T&& component) {
		// TODO: cleanup? undefined behaviour probably? ...
		//	might not be dropping the existing entity correctly...
		getComponent<T>(entity) = std::move(component);
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
		return _getPoolOrCreate<T>()->get<T>(entity);
	}

	template <ValidComponent T>
	bool Registry::hasComponent(Entity entity) {
		Signature const& signature = signatures.index(getIdentifier(entity));

		return signature.test(_getTypeIndex<T>());
	}

	template <OwnershipTag... Components>
	View<Components...> Registry::createView() {
		GroupMetadata* metadata = new GroupMetadata;
		metadata->registry = this;
		groups.push_back(metadata);

		metadata->create<Components...>();

		ComponentArray* iteratingArray = nullptr;
		uint64_t iteratingSize = std::numeric_limits<uint64_t>::max();

		bool ownedGroup = false;

		([&ownedGroup, &iteratingArray, &iteratingSize, metadata, this] {
			if constexpr (IsOwnedTag<Components>::value) {
				ownedGroup = true;

				uint32_t id = this->_getTypeIndex<Components::type>();
				ComponentArray* pool = this->componentPools[id];

				if (pool == nullptr) return;

				if (pool->size < iteratingSize) {
					iteratingSize = pool->size;
					iteratingArray = pool;
				}

				if (pool->isOwned()) {
					VIVIUM_LOG(LogSeverity::FATAL, "Couldn't create group, already owned by group");
				}

				pool->owner = metadata;
			}
			} (), ...);

		if (!ownedGroup) {
			([&iteratingArray, &iteratingSize, this] {
				uint32_t id = this->_getTypeIndex<Components::type>();
				ComponentArray* pool = this->componentPools[id];

				if (pool == nullptr) return;

				if (pool->size < iteratingSize) {
					iteratingSize = pool->size;
					iteratingArray = pool;
				}
				} (), ...);

			metadata->groupSize = iteratingArray->size;
		}

		for (uint64_t i = 0; i < iteratingSize; i++) {
			Entity& entity = iteratingArray->entities[i];
			Signature& signature = signatures.index(getIdentifier(entity));

			if (metadata->ownsSignature(signature)) {
				moveEntityIntoOwningGroup(entity, signature);
			}
		}

		return View<Components...> { this, iteratingArray->entities, metadata };
	}

	// Release ownership for affected pools
	template <OwnershipTag... Components>
	void Registry::destroyView(View<Components...> const& view) {
		// TODO
	}

	template <SerialiserInterface Interface>
	void serialiseWrite(Registry const& registry, Interface& interface) {
		// TODO: inconsistent registration order...?

		serialiseWrite(registry.componentPoolIndex, interface);
		serialiseWrite(registry.typeIndexMap.size(), interface);

		for (auto const& [key, index] : registry.typeIndexMap) {
			serialiseWrite(key, interface);
			serialiseWrite(index, interface);
		}

		serialiseWrite(registry.nextEntity, interface);
		serialiseWrite(registry.nextLargestEntity, interface);
		serialiseWrite(registry.availableEntities, interface);
		serialiseWrite(registry.entities, interface);

		// Write paged array
		//	we assume that the paged array will have the same page size and capacity between runs
		//	but this could only change between builds so its very safe

		serialiseWrite(registry.componentPools.size(), interface);

		for (uint64_t i = 0; i < registry.componentPools.size(); i++) {
			// TODO: only serialise if enabled/exists
			ComponentArray* componentArray = registry.componentPools[i];
			serialiseWrite(componentArray, interface);
		}

		serialiseWrite(registry.signatures, interface);
	}

	template <SerialiserInterface Interface>
	void serialiseRead(Registry* registry, Interface& interface) {
		serialiseRead(&registry->componentPoolIndex, interface);

		uint64_t typeIndexMapSize = 0;
		serialiseRead(&typeIndexMapSize, interface);

		for (uint64_t i = 0; i < typeIndexMapSize; i++) {
			uint64_t key, index;
			serialiseRead(&key, interface);
			serialiseRead(&index, interface);

			registry->typeIndexMap.insert({ key, index });
		}

		serialiseRead(&registry->nextEntity, interface);
		serialiseRead(&registry->nextLargestEntity, interface);
		serialiseRead(&registry->availableEntities, interface);
		serialiseRead(&registry->entities, interface);

		// Write paged array
		//	we assume that the paged array will have the same page size and capacity between runs
		//	but this could only change between builds so its very safe

		uint64_t componentPoolCount = 0;
		serialiseRead(&componentPoolCount, interface);

		for (uint64_t i = 0; i < componentPoolCount; i++) {
			// TODO: only serialise if enabled/exists
			// TODO: this won't work
			//	look in obsidian for notes on how to implement serialisation and deserialisation of the component array
			ComponentArray*& componentArray = registry->componentPools[i];
			serialiseRead(&componentArray, interface);
		}

		serialiseRead(&registry->signatures, interface);
	}
};