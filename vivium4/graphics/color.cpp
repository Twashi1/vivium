#include "color.h"

namespace Vivium {
	Color::Color(uint8_t r, uint8_t g, uint8_t b)
		: r(r / 255.0f), g(g / 255.0f), b(b / 255.0f)
	{}

	Color::Color(float r, float g, float b)
		: r(r), g(g), b(b)
	{}

	Color Color::multiply(Color color, float scalar)
	{
		return Color(color.r * scalar, color.g * scalar, color.b * scalar);
	}

	Color Color::White =	Color(1.0f, 1.0f, 1.0f);
	Color Color::Black =	Color(0.0f, 0.0f, 0.0f);
	Color Color::Gray =		Color(0.5f, 0.5f, 0.5f);
}