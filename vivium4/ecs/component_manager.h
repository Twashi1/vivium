#pragma once

#include <concepts>

#include "defines.h"
#include "../error/log.h"

namespace Vivium {
	template <typename T>
	concept ValidComponent = std::is_default_constructible_v<T> && (std::is_trivial_v<T> || std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>);

	template <ValidComponent T>
	void defaultMoveComponent(void* source, void* dest) {
		if constexpr (std::is_trivial_v<T> || std::is_copy_constructible_v<T>) {
			new (dest) T(*reinterpret_cast<T*>(source));
		}
		else if constexpr (std::is_move_constructible_v<T>) {
			new (dest) T(std::move(*reinterpret_cast<T*>(source)));
		}
	}

	template <ValidComponent T>
	void defaultReallocComponent(void* source, void* dest, uint64_t count) {
		for (uint64_t i = 0; i < count; i++) {
			defaultMoveComponent<T>(reinterpret_cast<T*>(source) + i, reinterpret_cast<T*>(dest) + i);
		}
	}

	template <ValidComponent T>
	void defaultDestroyComponent(void* data, uint64_t count) {
		for (uint64_t i = 0; i < count; i++) {
			(reinterpret_cast<T*>(data) + i)->~T();
		}
	}

	template <ValidComponent T>
	void defaultSwapComponent(void* a, void* b) {
		std::swap(*reinterpret_cast<T*>(a), *reinterpret_cast<T*>(b));
	}

	struct ComponentManager {
		typedef void(*MoveFunction)(void* src, void* dst);
		typedef void(*ReallocFunction)(void* src, void* dst, uint64_t);
		typedef void(*DestroyFunction)(void*, uint64_t);
		typedef void(*SwapFunction)(void*, void*);
		typedef uint32_t(*ComponentIDFunction)(void);

		MoveFunction moveFunction;
		ReallocFunction reallocFunction;
		DestroyFunction destroyFunction;
		SwapFunction swapFunction;
		ComponentIDFunction componentIDFunction;

		uint64_t typeSize;
	};

	template <ValidComponent T>
	ComponentManager defaultComponentManager() {
		ComponentManager manager;

		manager.moveFunction = defaultMoveComponent<T>;
		manager.reallocFunction = defaultReallocComponent<T>;
		manager.destroyFunction = defaultDestroyComponent<T>;
		manager.swapFunction = defaultSwapComponent<T>;
		manager.componentIDFunction = TypeGenerator::getIdentifier<T>;
		manager.typeSize = sizeof(T);

		return manager;
	}
}