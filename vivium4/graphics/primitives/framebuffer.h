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

	int getRequestedMultisamples(Engine::Handle engine, int multisampleCount);
		
	void dropFramebuffer(Framebuffer& framebuffer, Engine::Handle engine);

	// TODO: how to organise these, so we can also render to window
	// TODO: beginFrame/endFrame don't even require framebuffer at any point!!
	void beginFramebufferFrame(Framebuffer& framebuffer, CommandContext& context, Engine::Handle engine);
	void beginFramebufferRender(Framebuffer& framebuffer, CommandContext& context);
	// TODO: doesn't even take fraembuffer?
	void endFramebufferRender(Framebuffer& framebuffer, CommandContext& context);
	void endFramebufferFrame(Framebuffer& framebuffer, CommandContext& context, Engine::Handle engine);
}