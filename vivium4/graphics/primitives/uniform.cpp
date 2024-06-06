#include "uniform.h"

namespace Vivium {
	UniformData UniformData::fromBuffer(BufferReference buffer, uint64_t size, uint64_t offset)
	{
		UniformData data;

		data.bufferData = UniformData::BufferData(buffer, size, offset);

		return data;
	}

	UniformData UniformData::fromTexture(TextureReference texture)
	{
		UniformData data;

		data.textureData = UniformData::TextureData(texture);
				
		return data;
	}

	UniformData UniformData::fromFramebuffer(FramebufferReference framebuffer)
	{
		UniformData data;

		data.framebufferData = UniformData::FramebufferData(framebuffer);

		return data;
	}
		
	VkDescriptorType _descriptorType(UniformType type)
	{
		// TODO: different style of cast elsewhere
		return static_cast<VkDescriptorType>(static_cast<uint32_t>(type) & 0xffffffff);
	}
}