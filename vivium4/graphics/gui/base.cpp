#include "base.h"

namespace Vivium {
	namespace GUI {
		void Base::propagateUpdate(F32x2 windowDimensions, Base* parent)
		{
			F32x2 multiplier = F32x2(0.0f);

			switch (properties.scaleType) {
				case ScaleType::FIXED:		multiplier = F32x2(1.0f); break;
				case ScaleType::VIEWPORT:	multiplier = windowDimensions; break;
				case ScaleType::RELATIVE:	multiplier = parent->properties.trueDimensions; break;
				default: VIVIUM_LOG(Log::FATAL, "Invalid scale type, maybe not implemented?"); break;
			}

			properties.truePosition = properties.position * multiplier;

			if (properties.positionType == PositionType::RELATIVE)
				properties.truePosition += parent->properties.truePosition;

			properties.trueDimensions = properties.dimensions * multiplier;

			for (Base* child : children)
				child->propagateUpdate(windowDimensions, this);
		}
	}
}