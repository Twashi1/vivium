#include "descriptor_layout.h"

namespace Vivium {
	namespace DescriptorLayout {
		bool Resource::isNull() const
		{
			return layout == VK_NULL_HANDLE;
		}

		void Resource::drop(Engine::Handle engine)
		{
			vkDestroyDescriptorSetLayout(engine->device, layout, nullptr);
		}
	}
}