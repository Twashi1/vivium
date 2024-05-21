#pragma once

#include "../../engine.h"
#include "texture.h"

/*
TODO: framebuffers

Create render pass of some width/height, and suitable format

Create an image (w/ sample rate, color format, etc.)
Create an image view
Create a sampler

Create frame attachment (pure data)
Create color reference
Create subpass + subpass dependencies
Create render pass

Create framebuffer
Create descriptor?
*/

namespace Vivium {
	namespace Commands {
		namespace Context {
			struct Resource;
			typedef Resource* Handle;
		}
	}

	namespace Framebuffer {
		// TODO: dimensions and format not required?
		struct Resource {
			F32x2 dimensions;
			Texture::Format format;

			VkImage image;
			VkImageView view;
			VkSampler sampler;

			VkRenderPass renderPass;
			VkFramebuffer framebuffer;
		};

		struct Specification {
			F32x2 dimensions;
			Texture::Format format;
			int multisampleCount;
		};

		typedef Resource* Handle;
		typedef Resource* PromisedHandle;

		int getRequestedMultisamples(Engine::Handle engine, int multisampleCount);
		
		template <Allocator::AllocatorType AllocatorType>
		void drop(AllocatorType* allocator , Handle handle, Engine::Handle engine) {
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);
			VIVIUM_CHECK_HANDLE_EXISTS(handle);

			vkDestroyImage(engine->device, handle->image, nullptr);
			vkDestroySampler(engine->device, handle->sampler, nullptr);
			vkDestroyImageView(engine->device, handle->view, nullptr);

			vkDestroyRenderPass(engine->device, handle->renderPass, nullptr);
			vkDestroyFramebuffer(engine->device, handle->framebuffer, nullptr);

			Allocator::dropResource(allocator, handle);
		}

		void beginFrame(Handle handle, Commands::Context::Handle context, Engine::Handle engine);
		void beginRender(Handle handle, Commands::Context::Handle context);
		void endRender(Handle handle, Commands::Context::Handle context);
		void endFrame(Handle handle, Commands::Context::Handle context, Engine::Handle engine);
	}
}