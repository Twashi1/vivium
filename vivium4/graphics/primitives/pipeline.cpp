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

		void Resource::create(Engine::Handle engine, Window::Handle window, Specification specification)
		{
			// Graphics pipeline
			std::vector<VkPipelineShaderStageCreateInfo> shaderStages(specification.shaders.size());

			for (uint32_t i = 0; i < specification.shaders.size(); i++) {
				Shader::Handle shader = specification.shaders[i];

				VkPipelineShaderStageCreateInfo shaderStageInfo{};
				shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shaderStageInfo.stage = shader->flags;
				shaderStageInfo.module = shader->shader;
				shaderStageInfo.pName = "main";

				shaderStages[i] = shaderStageInfo;
			}

			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.vertexAttributeDescriptionCount = specification.bufferLayout.attributeDescriptions.size();
			vertexInputInfo.pVertexBindingDescriptions = &(specification.bufferLayout.bindingDescription);
			vertexInputInfo.pVertexAttributeDescriptions = specification.bufferLayout.attributeDescriptions.data();

			VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			VkPipelineViewportStateCreateInfo viewportState{};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.scissorCount = 1;

			VkPipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rasterizer.depthBiasEnable = VK_FALSE;

			VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = window->multisampleCount;

			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

			// TODO: possibly re-enable
			VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			// colorBlending.logicOpEnable = VK_FALSE;
			// colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
			// colorBlending.blendConstants[0] = 0.0f;
			// colorBlending.blendConstants[1] = 0.0f;
			// colorBlending.blendConstants[2] = 0.0f;
			// colorBlending.blendConstants[3] = 0.0f;

			std::vector<VkDynamicState> dynamicStates = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};

			VkPipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = dynamicStates.size();
			dynamicState.pDynamicStates = dynamicStates.data();

			std::vector<VkDescriptorSetLayout> descriptorLayouts(specification.descriptorLayouts.size());

			for (uint32_t i = 0; i < descriptorLayouts.size(); i++) {
				descriptorLayouts[i] = specification.descriptorLayouts[i]->layout;
			}

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();
			pipelineLayoutInfo.setLayoutCount = descriptorLayouts.size();
			// TODO: dirty, better to just make the copy
			pipelineLayoutInfo.pPushConstantRanges = reinterpret_cast<const VkPushConstantRange*>(specification.pushConstants.data());
			pipelineLayoutInfo.pushConstantRangeCount = specification.pushConstants.size();

			VIVIUM_VK_CHECK(vkCreatePipelineLayout(
				engine->device,
				&pipelineLayoutInfo,
				nullptr,
				&layout),
				"Failed to create pipeline layout"
			);

			VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = shaderStages.size();
			pipelineInfo.pStages = shaderStages.data();
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = nullptr;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.pDynamicState = &dynamicState;
			pipelineInfo.layout = layout;
			pipelineInfo.renderPass = engine->renderPass;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			pipelineInfo.basePipelineIndex = -1;

			VIVIUM_VK_CHECK(vkCreateGraphicsPipelines(
				engine->device,
				VK_NULL_HANDLE,
				1,
				&pipelineInfo,
				nullptr,
				&pipeline
			), "Failed to create graphics pipeline");
		}
		
		Specification::Specification(const std::span<const Shader::Handle> shaders, const Buffer::Layout bufferLayout, const std::span<const DescriptorLayout::Handle> descriptorLayouts, const std::span<const Uniform::PushConstant> pushConstants)
			: shaders(shaders), bufferLayout(bufferLayout), descriptorLayouts(descriptorLayouts), pushConstants(pushConstants)
		{}
	}
}