#include "framebuffer.h"
#include "../commands.h"
#include "../resource_manager.h"

namespace Vivium {
	int getRequestedMultisamples(Engine& engine, int multisampleCount)
	{
		// Verify valid multisample count
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(engine.physicalDevice, &deviceProperties);

		VkSampleCountFlags availableMultisampleCounts = deviceProperties.limits.framebufferColorSampleCounts;

		VIVIUM_ASSERT(
			(multisampleCount != 0) &&
			((multisampleCount & (multisampleCount - 1)) == 0) &&
			(multisampleCount <= 64),
			"Multisample count must be 1, 2, 4, ..., 64"
		);

		const std::array<VkSampleCountFlagBits, 6> possibleSampleCounts = {
			VK_SAMPLE_COUNT_64_BIT,
			VK_SAMPLE_COUNT_32_BIT,
			VK_SAMPLE_COUNT_16_BIT,
			VK_SAMPLE_COUNT_8_BIT,
			VK_SAMPLE_COUNT_4_BIT,
			VK_SAMPLE_COUNT_2_BIT
		};

		for (uint32_t i = 0; i < possibleSampleCounts.size(); i++) {
			// If the possible sample count is <= whats requested,
			// and available, use it (get next best option <, if best isn't available =)
			if (
				(possibleSampleCounts[i] <= multisampleCount) &&
				(availableMultisampleCounts & possibleSampleCounts[i])) {
				multisampleCount = possibleSampleCounts[i];

				break;
			}
		}

		return multisampleCount;
	}

	void beginFramebufferFrame(Framebuffer& framebuffer, CommandContext& context, Engine& engine)
	{
		// TODO: probably shouldn't be done here?
		glfwPollEvents();
		// TODO: fence might be useless?
		vkWaitForFences(engine.device, 1, &engine.inFlightFences[engine.currentFrameIndex], VK_TRUE, UINT64_MAX);
		vkResetFences(engine.device, 1, &engine.inFlightFences[engine.currentFrameIndex]);

		vkResetCommandBuffer(engine.commandBuffers[engine.currentFrameIndex], 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VIVIUM_VK_CHECK(vkBeginCommandBuffer(engine.commandBuffers[engine.currentFrameIndex], &beginInfo),
			"Failed to begin recording command buffer");
	}

	void beginFramebufferRender(Framebuffer& framebuffer, CommandContext& context)
	{
		// https://github.com/SaschaWillems/Vulkan/blob/master/examples/offscreen/offscreen.cpp
		VkClearValue clearValue;
		// TODO: customiseable
		clearValue.color = { { 0.0f, 0.0f, 0.0f, 0.0f } };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = framebuffer.renderPass;
		renderPassBeginInfo.framebuffer = framebuffer.framebuffer;
		renderPassBeginInfo.renderArea.extent.width = static_cast<uint32_t>(framebuffer.dimensions.x);
		renderPassBeginInfo.renderArea.extent.height = static_cast<uint32_t>(framebuffer.dimensions.y);
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(context.currentCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = framebuffer.dimensions.x;
		viewport.height = framebuffer.dimensions.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(context.currentCommandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.extent.width = static_cast<uint32_t>(framebuffer.dimensions.x);
		scissor.extent.height = static_cast<uint32_t>(framebuffer.dimensions.y);
		scissor.offset = { 0, 0 };
		vkCmdSetScissor(context.currentCommandBuffer, 0, 1, &scissor);
	}

	void endFramebufferRender(Framebuffer& framebuffer, CommandContext& context)
	{
		vkCmdEndRenderPass(context.currentCommandBuffer);
	}

	void endFramebufferFrame(Framebuffer& framebuffer, CommandContext& context, Engine& engine)
	{
		vkEndCommandBuffer(context.currentCommandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &engine.commandBuffers[engine.currentFrameIndex];

		VIVIUM_VK_CHECK(vkQueueSubmit(engine.graphicsQueue, 1, &submitInfo, engine.inFlightFences[engine.currentFrameIndex]),
			"Failed to submit draw command to buffer");

		engine.currentFrameIndex = (engine.currentFrameIndex + 1) % engine.MAX_FRAMES_IN_FLIGHT;

		// TODO: limit framerate maybe shouldn't be done here, since everything should happen in one frame
	}

	void dropFramebuffer(Framebuffer& framebuffer, Engine& engine) {
		vkDestroyImage(engine.device, framebuffer.image, nullptr);
		vkDestroySampler(engine.device, framebuffer.sampler, nullptr);
		vkDestroyImageView(engine.device, framebuffer.view, nullptr);

		vkDestroyRenderPass(engine.device, framebuffer.renderPass, nullptr);
		vkDestroyFramebuffer(engine.device, framebuffer.framebuffer, nullptr);
	}
}