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

		void Resource::create(Engine::Handle engine, Specification specification)
		{
			std::vector<VkDescriptorSetLayoutBinding> vulkanBindings(specification.bindings.size());
			bindings.resize(specification.bindings.size());

			for (uint64_t i = 0; i < specification.bindings.size(); i++) {
				VkDescriptorSetLayoutBinding& vulkanBinding = vulkanBindings[i];
				const Uniform::Binding& binding = specification.bindings[i];

				bindings[i] = binding;

				vulkanBinding.binding = binding.slot;
				vulkanBinding.descriptorCount = 1; // TODO: UBO arrays would require this to be editable
				vulkanBinding.descriptorType = static_cast<VkDescriptorType>(binding.type);
				vulkanBinding.stageFlags = static_cast<VkShaderStageFlags>(binding.stage);
				vulkanBinding.pImmutableSamplers = nullptr;
			}

			VkDescriptorSetLayoutCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			createInfo.bindingCount = vulkanBindings.size();
			createInfo.pBindings = vulkanBindings.data();

			vkCreateDescriptorSetLayout(engine->device, &createInfo, nullptr, &layout);
		}
		
		Specification::Specification(const std::span<const Uniform::Binding> bindings)
			: bindings(bindings)
		{}
	}
}