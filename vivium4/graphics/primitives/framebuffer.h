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

	namespace ResourceManager {
		namespace Static {
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
		
		template <Storage::StorageType StorageType>
		void drop(StorageType* allocator , Handle handle, Engine::Handle engine) {
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);
			VIVIUM_CHECK_HANDLE_EXISTS(handle);

			vkDestroyImage(engine->device, handle->image, nullptr);
			vkDestroySampler(engine->device, handle->sampler, nullptr);
			vkDestroyImageView(engine->device, handle->view, nullptr);

			vkDestroyRenderPass(engine->device, handle->renderPass, nullptr);
			vkDestroyFramebuffer(engine->device, handle->framebuffer, nullptr);

			Storage::dropResource(allocator, handle);
		}

		void drop(ResourceManager::Static::Handle manager, Framebuffer::Handle framebuffer, Engine::Handle engine);

		void beginFrame(Handle handle, Commands::Context::Handle context, Engine::Handle engine);
		void beginRender(Handle handle, Commands::Context::Handle context);
		void endRender(Handle handle, Commands::Context::Handle context);
		void endFrame(Handle handle, Commands::Context::Handle context, Engine::Handle engine);
	}
}