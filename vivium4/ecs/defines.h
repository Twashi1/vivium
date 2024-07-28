#pragma once

#include <bitset>
#include <cstdint>

namespace Vivium {
	constexpr uint64_t ECS_PAGE_SIZE = 4096U;

	constexpr uint32_t ECS_ENTITY_MAX = 0xfffff;
	constexpr uint32_t ECS_VERSION_MAX = 0xfff;
	constexpr uint32_t ECS_ENTITY_DEAD = 0xffffffff;

	constexpr uint8_t ECS_COMPONENT_MAX = 0xff;

	typedef uint32_t Entity;
	typedef std::bitset<ECS_COMPONENT_MAX> Signature;

	constexpr inline uint32_t getVersion(Entity entity) {
		return (entity & 0xfff00000) >> 20;
	}

	constexpr inline uint32_t getIdentifier(Entity entity) {
		return entity & ECS_ENTITY_MAX;
	}

	constexpr Entity nullEntity = getIdentifier(ECS_ENTITY_MAX);
	constexpr Entity tombstoneEntity = ECS_ENTITY_MAX & 0xfff00000;

	struct TypeGenerator {
		static uint32_t createIdentifier();
		
		template <typename>
		static uint32_t getIdentifier() {
			static const uint32_t value = createIdentifier();
			
			return value;
		}
	};
}