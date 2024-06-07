#pragma once

#include "shader.h"
#include "buffer.h"
#include "descriptor_layout.h"
#include "uniform.h"

namespace Vivium {
	enum _RenderTarget {
		FRAMEBUFFER,
		WINDOW
	};

	struct PipelineSpecification {
		std::vector<ShaderReference> shaders;
		BufferLayout bufferLayout;
		std::vector<DescriptorLayoutReference> descriptorLayouts;
		std::vector<PushConstant> pushConstants;
			
		_RenderTarget target;

		union {
			Engine::Handle engine;
			FramebufferReference framebuffer;
		};

		VkSampleCountFlagBits sampleCount;

		PipelineSpecification() = default;

		static PipelineSpecification fromWindow(
			const std::span<const ShaderReference> shaders,
			const BufferLayout& bufferLayout,
			const std::span<const DescriptorLayoutReference> descriptorLayouts,
			const std::span<const PushConstant> pushConstants,
			Engine::Handle engine,
			Window::Handle window
		);

		static PipelineSpecification fromFramebuffer(
			const std::span<const ShaderReference> shaders,
			const BufferLayout& bufferLayout,
			const std::span<const DescriptorLayoutReference> descriptorLayouts,
			const std::span<const PushConstant> pushConstants,
			FramebufferReference framebuffer,
			int multisampleCount
		);
	};

	struct Pipeline {
		VkPipelineLayout layout;
		VkPipeline pipeline;
		VkRenderPass renderPass;
	};

	struct PipelineReference {
		uint64_t referenceIndex;
	};

	template <Storage::StorageType StorageType>
	void drop(StorageType* allocator, Pipeline& pipeline, Engine::Handle engine)
	{
		vkDestroyPipelineLayout(engine->device, pipeline.layout, nullptr);
		vkDestroyPipeline(engine->device, pipeline.pipeline, nullptr);

		Storage::dropResource(allocator, &pipeline);
	}
}