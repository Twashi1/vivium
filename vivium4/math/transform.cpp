#include "transform.h"

namespace Vivium {
	Transform Transform::zero()
	{
		return Transform{ F32x2(0.0f), Mat2x2::identity(), Mat2x2::identity() };
	}
}