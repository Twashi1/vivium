#include "color.h"

namespace Vivium {
	Color Color::multiply(Color color, float scalar)
	{
		return Color(color.r * scalar, color.g * scalar, color.b * scalar);
	}

	Color Color::White =	Color(1.0f, 1.0f, 1.0f);
	Color Color::Black =	Color(0.0f, 0.0f, 0.0f);
	Color Color::Gray =		Color(0.5f, 0.5f, 0.5f);
}