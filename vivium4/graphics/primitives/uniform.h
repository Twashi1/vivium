#pragma once

#include "../../core.h"
#include "shader.h"
#include "buffer.h"
#include "texture.h"
#include "framebuffer.h"

namespace Vivium {
	namespace Uniform {
		enum class Type : uint64_t {
			UNIFORM_BUFFER			= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			STORAGE_BUFFER			= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			DYNAMIC_UNIFORM_BUFFER	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			TEXTURE					= (0Ui64 << 32) | VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			FRAMEBUFFER				= (1Ui64 << 32) | VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		};

		VkDescriptorType descriptorType(Type type);

		struct Binding {
			Shader::Stage stage;
			uint32_t slot;
			Type type;

			Binding() = default;
			Binding(Shader::Stage stage, uint32_t slot, Type type);
		};

		union Data {
			struct BufferData {
				Buffer::Handle buffer;
				uint64_t size;
				uint64_t offset;
			};

			struct DynamicBufferData {
				Buffer::Dynamic::Handle buffer;
				uint64_t size;
				uint64_t offset;
			};

			struct TextureData {
				Texture::Handle texture;
			};

			struct FramebufferData {
				Framebuffer::Handle framebuffer;
			};

			BufferData bufferData;
			TextureData textureData;
			FramebufferData framebufferData;
			DynamicBufferData dynamicBufferData;

			Data() = default;
				
			static Data fromBuffer(Buffer::Handle buffer, uint64_t size, uint64_t offset);
			// TODO: should be taking suballocation sizes in future
			//	and dealing with that appropriately when we create descriptor writes in resource manager
			static Data fromDynamicBuffer(Buffer::Dynamic::Handle buffer, uint64_t size, uint64_t offset);
			static Data fromTexture(Texture::Handle texture);
			static Data fromFramebuffer(Framebuffer::Handle framebuffer);
		};

		struct PushConstant {
			VkPushConstantRange range;

			PushConstant() = default;
			PushConstant(Shader::Stage stage, uint32_t size, uint32_t offset);
		};
	}
}