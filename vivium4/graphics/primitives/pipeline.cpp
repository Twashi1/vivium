#include "pipeline.h"

namespace Vivium {
	namespace Pipeline {
		bool Resource::isNull() const
		{
			return pipeline == VK_NULL_HANDLE;
		}

		void Resource::drop(Engine::Handle engine)
		{
			vkDestroyPipelineLayout(engine->device, layout, nullptr);
			vkDestroyPipeline(engine->device, pipeline, nullptr);
		}
		
		Specification::Specification(const std::span<const Shader::Handle> shaders, const Buffer::Layout bufferLayout, const std::span<const DescriptorLayout::Handle> descriptorLayouts, const std::span<const Uniform::PushConstant> pushConstants)
			: shaders(shaders), bufferLayout(bufferLayout), descriptorLayouts(descriptorLayouts), pushConstants(pushConstants)
		{}
	}
}