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
		
	char const* getString(UniformType type)
	{
		switch (type) {
		case UniformType::UNIFORM_BUFFER: return "UniformBuffer";
		case UniformType::DYNAMIC_UNIFORM_BUFFER: return "DynamicUniformBuffer";
		case UniformType::STORAGE_BUFFER: return "StorageBuffer";
		case UniformType::TEXTURE: return "Texture";
		case UniformType::FRAMEBUFFER: return "Framebuffer";
		default: return "Unknown";
		}
	}

	VkDescriptorType _descriptorType(UniformType type)
	{
		// TODO: different style of cast elsewhere
		return static_cast<VkDescriptorType>(static_cast<uint32_t>(type) & 0xffffffff);
	}
}