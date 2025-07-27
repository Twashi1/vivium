#pragma once

#include <concepts>

#include "defines.h"
#include "../error/log.h"
#include "../serialiser/serialiser.h"

namespace Vivium {
	// TODO: better
	template <typename T>
	concept ValidComponent = true;

	template <ValidComponent T>
	void defaultMoveComponent(void* source, void* dest);

	template <ValidComponent T>
	void defaultReallocComponent(void* source, void* dest, uint64_t count);

	template <ValidComponent T>
	void defaultDestroyComponent(void* data, uint64_t count);

	template <ValidComponent T>
	void defaultSwapComponent(void* a, void* b);

	template <typename T>
	void defaultSerialiseWrite(void const* src, SerialiserMemoryInterface& store);

	template <typename T>
	void defaultSerialiseRead(void* src, SerialiserMemoryInterface& store);

	struct ComponentManager {
		typedef void(*MoveFunction)(void* src, void* dst);
		typedef void(*SerialiseWrite)(void const* src, SerialiserMemoryInterface& store);
		typedef void(*SerialiseRead)(void* src, SerialiserMemoryInterface& store);
		typedef void(*ReallocFunction)(void* src, void* dst, uint64_t);
		typedef void(*DestroyFunction)(void*, uint64_t);
		typedef void(*SwapFunction)(void*, void*);

		MoveFunction moveFunction = nullptr;
		SerialiseWrite writeFunction = nullptr;
		SerialiseRead readFunction = nullptr;
		ReallocFunction reallocFunction = nullptr;
		DestroyFunction destroyFunction = nullptr;
		SwapFunction swapFunction = nullptr;

		uint64_t componentTypeIndex;
		uint64_t typeSize;

		uint64_t getOffset(uint64_t index) const;
	};

	template <typename T>
	ComponentManager defaultComponentManager(uint64_t componentTypeIndex);
}