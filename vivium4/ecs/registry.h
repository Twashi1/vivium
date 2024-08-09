#pragma once

#include "paged_array.h"
#include "defines.h"
#include "component_array.h"

#include "../error/log.h"

namespace Vivium {
	struct Registry;

	template <typename T, OwnershipTag... Components>
	constexpr inline bool _isOwnedType = (std::is_same_v<T, typename Components::type> || ...);

	// https://internalpointers.com/post/writing-custom-iterators-modern-cpp
	template <OwnershipTag... Components>
	struct ViewElement {
		uint64_t index;
		Entity entity;

		Registry* registry;

		template <typename T>
		T& get() {
			if constexpr (_isOwnedType<T, Components...>) {
				return registry->componentPools[TypeGenerator::getIdentifier<T>()]->_getIndex<T>(index);
			}
			else {
				return registry->getComponent<T>(entity);
			}
		}
	};

	template <OwnershipTag... WrappedTypes>
	struct View {
		Registry* registry;
		Entity* ownedEntityArray;
		GroupMetadata* groupMetadata;

		struct ViewIterator {
			using iterator_category = std::forward_iterator_tag;
			using difference_type = void;
			using value_type = ViewElement<WrappedTypes...>;
			using pointer = value_type*;
			using reference = value_type&;

			Registry* registry;
			Entity* ownedEntityArray;
			GroupMetadata* groupMetadata;

			value_type current;

			ViewIterator(Registry* registry, Entity* ownedEntityArray, GroupMetadata* groupMetadata, uint64_t startIndex, Entity entity)
				: registry(registry), ownedEntityArray(ownedEntityArray), groupMetadata(groupMetadata)
			{
				current.index = startIndex;
				current.entity = entity;
				current.registry = registry;
			}

			reference operator*() { return current; }
			pointer operator->() { return &current; }

			ViewIterator& operator++() {
				current.entity = ownedEntityArray[current.index++];

				// All partially owned
				// TODO: determine at compile-time
				if (!groupMetadata->ownedComponents.any()) {
					while (groupMetadata->containsSignature(registry->signatures.get(getIdentifier(current.entity))) && current.index < groupMetadata->groupSize) {
						current.entity = ownedEntityArray[current.index++];
					}
				}

				return *this;
			}
			ViewIterator operator++(int) { ViewIterator tmp = *this; ++(*this); return tmp; }

			bool operator==(ViewIterator const& other) { return current.index == other.current.index; }
			bool operator!=(ViewIterator const& other) { return current.index != other.current.index; }
		};

		ViewIterator begin() { return ViewIterator(registry, ownedEntityArray, groupMetadata, 0, ownedEntityArray[0]); }
		ViewIterator end() { return ViewIterator(registry, ownedEntityArray, groupMetadata, groupMetadata->groupSize, ECS_ENTITY_DEAD); }
	};

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

			if (arr->owner != nullptr && arr->owner->ownsSignature(signature)) {
				moveEntityIntoOwningGroup(entity, signature);
			}
		}

		template <ValidComponent T>
		void removeComponent(Entity entity) {
			uint8_t componentID = TypeGenerator::getIdentifier<T>();
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
		T& getComponent(Entity entity) {
			return _getPoolOrCreate<T>()->get<T>(entity);
		}

		template <OwnershipTag... Components>
		View<Components...> createView() {
			GroupMetadata* metadata = new GroupMetadata;
			groups.push_back(metadata);

			metadata->create<Components...>();

			ComponentArray* iteratingArray = nullptr;
			uint64_t iteratingSize = std::numeric_limits<uint64_t>::max();

			bool ownedGroup = false;

			([&ownedGroup, &iteratingArray, &iteratingSize, metadata, this] {
				if constexpr (IsOwnedTag<Components>::value) {
					ownedGroup = true;

					uint32_t id = TypeGenerator::getIdentifier<Components::type>();
					ComponentArray* pool = this->componentPools[id];

					if (pool == nullptr) return;

					if (pool->size < iteratingSize) {
						iteratingSize = pool->size;
						iteratingArray = pool;
					}

					if (pool->isOwned()) {
						VIVIUM_LOG(Log::FATAL, "Couldn't create group, already owned by group");
					}

					pool->owner = metadata;
				}
			} (), ...);

			if (!ownedGroup) {
				([&iteratingArray, &iteratingSize, this] {
					uint32_t id = TypeGenerator::getIdentifier<Components::type>();
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
		void destroyView(View<Components...> const& view) {
			// TODO
		}
	};
}