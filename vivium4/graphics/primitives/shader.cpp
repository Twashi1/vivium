#include "shader.h"

namespace Vivium {
	namespace Shader {
		Stage operator|(Stage lhs, Stage rhs)
		{
			return static_cast<Stage>(static_cast<int>(lhs) | static_cast<int>(rhs));
		}

		uint32_t sizeOf(DataType type)
		{
			return static_cast<uint32_t>(static_cast<uint64_t>(type) >> 32Ui64);
		}
			
		VkFormat formatOf(DataType type)
		{
			return static_cast<VkFormat>(static_cast<uint32_t>(type));
		}
	}
}