#pragma once

#include "../../core.h"
#include "shader.h"
#include "buffer.h"
#include "texture.h"

namespace Vivium {
	namespace Uniform {
		enum class Type {
			BUFFER = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			STORAGE = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			BUFFER_DYNAMIC = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			TEXTURE = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		};

		struct Binding {
			Shader::Stage stage;
			uint32_t slot;
			Type type;
		};

		union Data {
			struct BufferData {
				Buffer::Handle buffer;
				uint64_t size;
				uint64_t offset;
			};

			struct TextureData {
				Texture::Handle texture;
			};

			BufferData bufferData;
			TextureData textureData;

			Data() = default;
				
			static Data fromBuffer(Buffer::Handle buffer, uint64_t size, uint64_t offset);
			static Data fromTexture(Texture::Handle texture);
		};

		struct PushConstant {
			VkPushConstantRange range;

			PushConstant() = default;
			PushConstant(Shader::Stage stage, uint32_t size, uint32_t offset);
		};
	}
}