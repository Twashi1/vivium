#pragma once

#include "../../core.h"
#include "buffer.h"
#include "framebuffer.h"
#include "shader.h"
#include "texture.h"

namespace Vivium {
// TODO: change how this system works
enum class UniformType : uint64_t {
  UNIFORM_BUFFER = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
  STORAGE_BUFFER = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
  DYNAMIC_UNIFORM_BUFFER = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
  TEXTURE = (0ULL << 32) | VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
  FRAMEBUFFER = (1ULL << 32) | VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
};

char const* getString(UniformType type);
VkDescriptorType _descriptorType(UniformType type);

struct UniformBinding {
  ShaderStage stage;
  uint32_t slot;
  UniformType type;
};

// TODO: get rid of member structus
//	how can we tell apart? does the descriptor layout tell us?
union UniformData {
  struct BufferData {
    BufferReference buffer;
    uint64_t size;
    uint64_t offset;
  };

  struct TextureData {
    TextureReference texture;
  };

  struct FramebufferData {
    FramebufferReference framebuffer;
  };

  BufferData bufferData;
  TextureData textureData;
  FramebufferData framebufferData;

  static UniformData fromBuffer(BufferReference buffer, uint64_t size,
                                uint64_t offset);
  static UniformData fromTexture(TextureReference texture);
  static UniformData fromFramebuffer(FramebufferReference framebuffer);
};

// TODO: offset before size!
struct PushConstant {
  ShaderStage stage;
  uint32_t offset;
  uint32_t size;
};
}  // namespace Vivium
