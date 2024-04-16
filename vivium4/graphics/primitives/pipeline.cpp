#include "pipeline.h"

namespace Vivium {
	namespace Pipeline {
		bool isNull(const Handle pipeline)
		{
			return pipeline->pipeline == VK_NULL_HANDLE;
		}
		
		Specification::Specification(const std::span<const Shader::Handle> shaders, const Buffer::Layout bufferLayout, const std::span<const DescriptorLayout::Handle> descriptorLayouts, const std::span<const Uniform::PushConstant> pushConstants)
			: shaders(shaders), bufferLayout(bufferLayout), descriptorLayouts(descriptorLayouts), pushConstants(pushConstants)
		{}
	}
}