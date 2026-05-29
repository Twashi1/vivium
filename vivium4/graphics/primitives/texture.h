#pragma once

#include <type_traits>

#include "../../core.h"
#include "../../engine.h"
#include "../gui/font.h"
#include "../image_load.h"
#include "../texture_format.h"

namespace Vivium {
enum class TextureFilter : std::underlying_type_t<VkFilter> {
  NEAREST = VK_FILTER_NEAREST,
  LINEAR = VK_FILTER_LINEAR
};

std::string getString(TextureFilter filter);

struct Texture {
  VkImage image;
  VkImageView view;
  VkSampler sampler;
};

struct TextureSpecification {
  int width, height, channels;

  std::vector<uint8_t> data;

  TextureFormat imageFormat;
  TextureFilter imageFilter;

  // TODO: from raw data
  static TextureSpecification fromImageFile(const char* imageFile,
                                            TextureFormat imageFormat,
                                            TextureFilter imageFilter);
  static TextureSpecification fromFont(Font const& font,
                                       TextureFormat imageFormat,
                                       TextureFilter imageFilter);
  static TextureSpecification fromData(uint8_t const* data, I32x2 dimensions,
                                       TextureFormat imageFormat,
                                       TextureFilter imageFilter);
  static TextureSpecification fromImage(Image image, TextureFilter imageFilter);
};

struct TextureReference {
  uint64_t referenceIndex;
};

void dropTexture(Texture& texture, Engine& engine);
}  // namespace Vivium
