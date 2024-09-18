#pragma once

#include <cstdint>

namespace Vivium {
	struct Color {
		float r, g, b;

		Color() = default;
		constexpr Color(uint8_t r, uint8_t g, uint8_t b)
			:
			r(static_cast<float>(r / 255.0f)),
			g(static_cast<float>(g / 255.0f)),
			b(static_cast<float>(b / 255.0f))
		{}
		constexpr Color(float r, float g, float b) : r(r), g(g), b(b) {}

		static Color multiply(Color color, float scalar);
	};
}