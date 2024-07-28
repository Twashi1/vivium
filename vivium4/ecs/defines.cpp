#include "defines.h"

namespace Vivium {
	uint32_t TypeGenerator::createIdentifier()
	{
		static uint32_t value = 0;

		return value++;
	}
}