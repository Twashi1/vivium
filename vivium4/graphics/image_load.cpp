#include "image_load.h"

namespace Vivium {
Image loadImage(char const* filename, TextureFormat format) {
  Image image;

  int stbi_format;

  switch (format) {
    case TextureFormat::RGBA:
      stbi_format = STBI_rgb_alpha;
      break;
    case TextureFormat::MONOCHROME:
      stbi_format = STBI_grey;
      break;
    default:
      stbi_format = STBI_default;

      VIVIUM_LOG(LogSeverity::FATAL, "Invalid image format");

      break;
  }

  int channels = 0;

  uint8_t* data =
      stbi_load(filename, &image.size.x, &image.size.y, &channels, stbi_format);
  VIVIUM_ASSERT(data != nullptr, "Failed to load image file");
  VIVIUM_ASSERT(channels > 0, "Failed to load number of channels");
  uint64_t imageSize = static_cast<uint64_t>(image.size.x) *
                       static_cast<uint64_t>(image.size.y) *
                       static_cast<uint64_t>(channels);

  // Copy image data into specification
  VIVIUM_LOG(
      LogSeverity::DEBUG,
      "Allocating image buffer of size {}, for img {}x{} with {} channels",
      imageSize, image.size.x, image.size.y, channels);
  image.data = reinterpret_cast<uint8_t*>(std::malloc(imageSize));
  VIVIUM_ASSERT(image.data != nullptr, "Failed to allocate space for image");

  std::memcpy(image.data, data, imageSize);

  image.format = format;

  stbi_image_free(data);

  VIVIUM_LOG(LogSeverity::DEBUG, "Didn't freeze on stbi image free");

  return image;
}

void dropImage(Image& image) {
  if (image.data == nullptr) return;

  std::free(image.data);
  VIVIUM_DEBUG_ONLY(image.data = nullptr);
}
}  // namespace Vivium
