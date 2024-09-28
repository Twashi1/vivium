#include "descriptor_layout.h"

namespace Vivium {
	void dropDescriptorLayout(DescriptorLayout& layout, Engine& engine)
	{
		vkDestroyDescriptorSetLayout(engine.device, layout.layout, nullptr);
	}
}