#include "buffer.h"
#include "engine.h"

namespace Vivium {
	namespace Graphics {
		namespace Buffer {
			bool Resource::isNull() const
			{
				return buffer == nullptr;
			}
			
			void Resource::close()
			{
				vkDestroyBuffer(buffer, nullptr);
			}
		}
	}
}