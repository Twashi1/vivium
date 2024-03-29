#pragma once

#include "../../engine.h"
#include "uniform.h"

namespace Vivium {
	namespace DescriptorLayout {
		struct Resource {
			VkDescriptorSetLayout layout;
			std::vector<Uniform::Binding> bindings;

			bool isNull() const;
			void drop(Engine::Handle engine);
		};
		
		typedef Resource* Handle;

		struct Specification {
			const std::span<const Uniform::Binding> bindings;
		};
	}
}