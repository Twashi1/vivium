#include "defines.h"

namespace Vivium {
	uint32_t getVersion(Entity entity) {
		return (entity & ECS_VERSION_MASK) >> ECS_VERSION_SHIFT;
	}

	uint32_t getIdentifier(Entity entity) {
		return entity & ECS_ENTITY_MASK;
	}

	uint32_t TypeGenerator::createIdentifier()
	{
		static uint32_t value = 0;

		return value++;
	}
}