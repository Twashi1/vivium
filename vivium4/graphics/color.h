#pragma once

#include <cstdint>

namespace Vivium {
	struct Color {
		float r, g, b;

		Color() = default;
		Color(uint8_t r, uint8_t g, uint8_t b);
		Color(float r, float g, float b);

		static Color White;
		static Color Black;
		static Color Gray;
	};
}