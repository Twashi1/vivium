#pragma once

#include "component_manager.h"
#include "defines.h"
#include "paged_array.h"
#include "../error/log.h"
#include "group.h"

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
		GroupMetadata* owner;

		bool requiresDeserialise;

		ComponentArray();
		~ComponentArray();

		void _allocateForIndex(uint64_t index);

		void resize(uint64_t newCapacity);
		bool contains(Entity entity);
		void swap(Entity a, Entity b);
		void free(Entity entity);
		void clear();

		template <ValidComponent T>
		void deserialise(uint64_t componentTypeIndex) {
			// Checking for valid use
			VIVIUM_ASSERT(dense != nullptr, "Can't deserialise if data hasn't been loaded");
			VIVIUM_ASSERT(requiresDeserialise, "Doesn't require deserialisation, misconfiguration or loaded improperly?");

			// sparse/entities/size already in
			manager = defaultComponentManager<T>(componentTypeIndex);
			// Create new component data array
			uint8_t* components = new uint8_t[size * manager.typeSize];
			// Begin deserialising components and writing it to the new array
			// Create a memory interface to the dense array to read from
			// TODO: bad... should be some dedicated serialiser interface for memory you
			//	already own?
			SerialiserMemoryInterface store;
			store.destination = dense;
			store.offset = 0;
			store.maxSize = capacity;

			for (uint64_t i = 0; i < size; i++) {
				manager.readFunction(components + manager.getOffset(i), store);
			}

			// Replace the dense array with our own now
			delete[] dense;
			dense = components;
			capacity = size * manager.typeSize;
			requiresDeserialise = false;
		}

		bool isOwned() const;

		template <ValidComponent T>
		void push(Entity entity, T&& component) {
			if (sparse.get(getIdentifier(entity)) != ECS_ENTITY_DEAD) {
				VIVIUM_LOG(LogSeverity::FATAL, "Entity already had component");

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
				VIVIUM_LOG(LogSeverity::FATAL, "Entity didn't have component");
			}

			return *reinterpret_cast<T*>(&dense[index * manager.typeSize]);
		}

		template <ValidComponent T>
		T& _getIndex(uint64_t index) {
			return *reinterpret_cast<T*>(&dense[index * manager.typeSize]);
		}

		template <ValidComponent T>
		T const& _getIndex(uint64_t index) const {
			return *reinterpret_cast<T const*>(&dense[index * manager.typeSize]);
		}
	};

	template <SerialiserInterface Interface>
	void serialiseWrite(ComponentArray const& componentArray, Interface& interface) {
		serialiseWrite(componentArray.sparse, interface);
		serialiseWrite(componentArray.size, interface);
		// TODO: best we can do?...
		interface.writeBytes(componentArray.size * sizeof(Entity), componentArray.entities);

		SerialiserMemoryInterface memoryInterface;
		memoryInterface.begin(componentArray.size * componentArray.manager.typeSize);

		for (uint64_t i = 0; i < componentArray.size; i++) {
			void* componentData = componentArray.dense + componentArray.manager.getOffset(i);
			componentArray.manager.writeFunction(componentData, memoryInterface);
		}

		// Write memory interface to regular interface
		serialiseWrite(memoryInterface.offset, interface);
		interface.writeBytes(memoryInterface.offset, memoryInterface.destination);

		memoryInterface.end();

		// Not serialising the component manager or the group metadata
	}

	template <SerialiserInterface Interface>
	void serialiseRead(ComponentArray* componentArray, Interface& interface) {
		dispatchSerialiseRead(&componentArray->sparse, interface);
		dispatchSerialiseRead(&(componentArray->size), interface);
		componentArray->entities = new Entity[componentArray->size];
		interface.readBytes(componentArray->size * sizeof(Entity), componentArray->entities);

		// We allocate enough space to store n elements, although this
		//	might not necessarily accommodate all the deserialised data
		//	its a good starting point

		// Number of bytes of all the component data
		uint64_t totalSize = 0;
		serialiseRead(&totalSize, interface);
		// We're gonna hijack the component array in a couple ways
		//	reusing some memory... very bad!
		componentArray->dense = new uint8_t[totalSize];
		interface.readBytes(totalSize, componentArray->dense);
		componentArray->capacity = totalSize; // So we know the size of the array for future reference
		componentArray->manager = ComponentManager();

		componentArray->requiresDeserialise = true;
	}
}