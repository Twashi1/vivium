#pragma once

#include <cstdint>

namespace Vivium {
struct Color {
  float r, g, b;

  Color() = default;
  constexpr Color(uint8_t r, uint8_t g, uint8_t b)
      : r(static_cast<float>(r / 255.0f)),
        g(static_cast<float>(g / 255.0f)),
        b(static_cast<float>(b / 255.0f)) {}
  constexpr Color(float r, float g, float b) : r(r), g(g), b(b) {}

  static Color multiply(Color color, float scalar);
};

struct ColorAlpha {
  float r, g, b, a;

  ColorAlpha() = default;
  constexpr ColorAlpha(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
      : r(static_cast<float>(r / 255.0f)),
        g(static_cast<float>(r / 255.0f)),
        b(static_cast<float>(r / 255.0f)),
        a(static_cast<float>(r / 255.0f)) {}
  constexpr ColorAlpha(float r, float g, float b, float a)
      : r(r), g(g), b(b), a(a) {}
};
}  // namespace Vivium
