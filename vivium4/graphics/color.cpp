#include "color.h"

namespace Vivium {
	Color Color::multiply(Color color, float scalar)
	{
		return Color(color.r * scalar, color.g * scalar, color.b * scalar);
	}
}