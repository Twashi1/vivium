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
	}
}