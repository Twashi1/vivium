#pragma once

#include "../../engine.h"
#include "texture.h"

namespace Vivium {
	namespace Commands {
		namespace Context {
			struct Resource;

			typedef Resource* Handle;
		}
	}

	// TODO: dimensions and format not required?
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
		
	template <Storage::StorageType StorageType>
	void drop(StorageType* allocator, Framebuffer& framebuffer, Engine::Handle engine) {
		vkDestroyImage(engine->device, framebuffer.image, nullptr);
		vkDestroySampler(engine->device, framebuffer.sampler, nullptr);
		vkDestroyImageView(engine->device, framebuffer.view, nullptr);

		vkDestroyRenderPass(engine->device, framebuffer.renderPass, nullptr);
		vkDestroyFramebuffer(engine->device, framebuffer.framebuffer, nullptr);

		Storage::dropResource(allocator, &framebuffer);
	}

	// TODO: how to organise these, so we can also render to window
	// TODO: beginFrame/endFrame don't even require framebuffer at any point!!
	void beginFramebufferFrame(Framebuffer& framebuffer, Commands::Context::Handle context, Engine::Handle engine);
	void beginFramebufferRender(Framebuffer& framebuffer, Commands::Context::Handle context);
	// TODO: doesn't even take fraembuffer?
	void endFramebufferRender(Framebuffer& framebuffer, Commands::Context::Handle context);
	void endFramebufferFrame(Framebuffer& framebuffer, Commands::Context::Handle context, Engine::Handle engine);
}