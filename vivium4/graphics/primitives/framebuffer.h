#pragma once

#include "../../engine.h"
#include "texture.h"

namespace Vivium {
	struct CommandContext;

	struct Framebuffer {
		VkImage image;
		VkImageView view;
		VkSampler sampler;

		VkRenderPass renderPass;
		VkFramebuffer framebuffer;

		VkCommandPool commandPool;
		std::array<VkCommandBuffer, VIVIUM_FRAMES_IN_FLIGHT> commandBuffers;

		std::array<VkSemaphore, VIVIUM_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
		std::array<VkSemaphore, VIVIUM_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
		std::array<VkFence, VIVIUM_FRAMES_IN_FLIGHT> inFlightFences;

		uint32_t currentFrameIndex;

		U32x2 dimensions;
	};

	struct FramebufferSpecification {
		U32x2 dimensions;
		TextureFormat format;
		int multisampleCount;
	};

	struct FramebufferReference {
		uint64_t referenceIndex;
	};

	// TODO: better place for this function, also private
	int getRequestedMultisamples(Engine& engine, int multisampleCount);
		
	void dropFramebuffer(Framebuffer& framebuffer, Engine& engine);

	// TODO: lots of overlap in functionality with window, could abstract
	void beginFramebufferFrame(Framebuffer& framebuffer, CommandContext& context, Engine& engine);
	void beginFramebufferRender(Framebuffer& framebuffer, CommandContext& context);
	void endFramebufferRender(Framebuffer& framebuffer, CommandContext& context);
	void endFramebufferFrame(Framebuffer& framebuffer, CommandContext& context, Engine& engine);
}