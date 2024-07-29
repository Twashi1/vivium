#pragma once

#include <bitset>
#include <cstdint>

namespace Vivium {
	constexpr uint64_t ECS_PAGE_SIZE = 4096U;

	constexpr uint32_t ECS_ENTITY_MAX = 0xfffff;
	constexpr uint32_t ECS_VERSION_MAX = 0xfff;
	constexpr uint32_t ECS_ENTITY_DEAD = 0xffffffff;

	constexpr uint32_t ECS_VERSION_MASK = 0xfff00000;
	constexpr uint32_t ECS_VERSION_SHIFT = 20;
	constexpr uint32_t ECS_ENTITY_MASK = 0x000fffff;

	constexpr uint8_t ECS_COMPONENT_MAX = 0xff;

	typedef uint32_t Entity;
	typedef std::bitset<ECS_COMPONENT_MAX> Signature;

	uint32_t getVersion(Entity entity);
	uint32_t getIdentifier(Entity entity);

	// TODO: use both
	constexpr Entity nullEntity = ECS_ENTITY_MAX & ECS_ENTITY_MASK;
	constexpr Entity tombstoneEntity = ECS_ENTITY_MAX & ECS_VERSION_MASK;

	struct TypeGenerator {
		static uint32_t createIdentifier();
		
		template <typename>
		static uint32_t getIdentifier() {
			static const uint32_t value = createIdentifier();
			
			return value;
		}
	};
}