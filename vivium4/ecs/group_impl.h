#pragma once

#include "group.h"

namespace Vivium {
	template <OwnershipTag... WrappedTypes>
	void GroupMetadata::create() {
		groupSize = 0;

		// Setup signatures
		([this]() {
			// TODO: need the registry for this
			uint8_t id = registry->_getTypeIndex<typename WrappedTypes::type>();

			affectedComponents.set(id);

			if constexpr (IsOwnedTag<WrappedTypes>::value) {
				ownedComponents.set(id);
			}
			else {
				partialComponents.set(id);
			}
			} (), ...);
	}

	template <typename T>
	bool GroupMetadata::contains() { return affectedComponents.test(registry->_getTypeIndex<T>()); }

	template <typename... Ts>
	bool GroupMetadata::any() { return (contains<Ts>() || ...); }
	template <typename... Ts>
	bool GroupMetadata::all() { return (contains<Ts>() && ...); }

	// https://internalpointers.com/post/writing-custom-iterators-modern-cpp
	template <OwnershipTag... Components>
	template <typename T>
	T& ViewElement<Components...>::get() {
		if constexpr (_isOwnedType<T, Components...>) {
			// TODO: getOrCreate pool?
			uint64_t poolIndex = registry->_getTypeIndex<T>();
			ComponentArray* arr = (registry->componentPools)[poolIndex];
			return arr->_getIndex<T>(index);
		}
		else {
			return registry->getComponent<T>(entity);
		}
	}

	template <OwnershipTag... Components>
	template <typename T>
	T const& ViewElement<Components...>::get() const {
		if constexpr (_isOwnedType<T, Components...>) {
			// TODO: getOrCreate pool?
			uint64_t poolIndex = registry->_getTypeIndex<T>();
			ComponentArray const* arr = (registry->componentPools)[poolIndex];
			return arr->_getIndex<T>(index);
		}
		else {
			return registry->getComponent<T>(entity);
		}
	}

	template <OwnershipTag... WrappedTypes>
	View<WrappedTypes...>::ViewIterator::ViewIterator(Registry* registry, Entity* ownedEntityArray, GroupMetadata* groupMetadata, uint64_t startIndex, Entity entity)
		: registry(registry), ownedEntityArray(ownedEntityArray), groupMetadata(groupMetadata)
	{
		current.index = startIndex;
		current.entity = entity;
		current.registry = registry;
	}

	template <OwnershipTag... WrappedTypes>
	View<WrappedTypes...>::ViewIterator::reference View<WrappedTypes...>::ViewIterator::operator*() { return current; }
	template <OwnershipTag... WrappedTypes>
	View<WrappedTypes...>::ViewIterator::pointer View<WrappedTypes...>::ViewIterator::operator->() { return &current; }

	template <OwnershipTag... WrappedTypes>
	struct View<WrappedTypes...>::ViewIterator& View<WrappedTypes...>::ViewIterator::operator++()
	{
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

	template <OwnershipTag... WrappedTypes>
	View<WrappedTypes...>::ViewIterator View<WrappedTypes...>::ViewIterator::operator++(int) {
		ViewIterator tmp = *this; ++(*this); return tmp;
	}

	template <OwnershipTag... WrappedTypes>
	bool View<WrappedTypes...>::ViewIterator::operator==(ViewIterator const& other) { return current.index == other.current.index; }
	
	template <OwnershipTag... WrappedTypes>
	bool View<WrappedTypes...>::ViewIterator::operator!=(ViewIterator const& other) { return current.index != other.current.index; }

	template <OwnershipTag... WrappedTypes>
	View<WrappedTypes...>::ViewIterator View<WrappedTypes...>::begin()
	{
		return ViewIterator(registry, ownedEntityArray, groupMetadata, 0, ownedEntityArray[0]);
	}
	
	template <OwnershipTag... WrappedTypes>
	View<WrappedTypes...>::ViewIterator View<WrappedTypes...>::end()
	{
		return ViewIterator(registry, ownedEntityArray, groupMetadata, groupMetadata->groupSize, ECS_ENTITY_DEAD);
	}
}