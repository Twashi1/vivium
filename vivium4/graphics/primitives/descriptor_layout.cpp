#include "descriptor_layout.h"

namespace Vivium {
	void dropDescriptorLayout(DescriptorLayout& layout, Engine::Handle engine)
	{
		vkDestroyDescriptorSetLayout(engine->device, layout.layout, nullptr);
	}
}