#include "uniform.h"

namespace Vivium {
	namespace Uniform {
		Data Data::fromBuffer(Buffer::Handle buffer, uint64_t size, uint64_t offset)
		{
			Data data;

			data.bufferData = Data::BufferData(buffer, size, offset);

			return data;
		}

		Data Data::fromDynamicBuffer(Buffer::Dynamic::Handle buffer, uint64_t size, uint64_t offset)
		{
			Data data;

			data.dynamicBufferData = Data::DynamicBufferData(buffer, size, offset);

			return data;
		}

		Data Data::fromTexture(Texture::Handle texture)
		{
			Data data;

			data.textureData = Data::TextureData(texture);
				
			return data;
		}
		
		PushConstant::PushConstant(Shader::Stage stage, uint32_t size, uint32_t offset)
		{
			range = VkPushConstantRange{};
			range.stageFlags = static_cast<VkShaderStageFlags>(stage);
			range.offset = offset;
			range.size = size;
		}
		
		Binding::Binding(Shader::Stage stage, uint32_t slot, Type type)
			: stage(stage), slot(slot), type(type)
		{}
	}
}