#pragma once

#include "base.h"

namespace Vivium {
	namespace GUI {
		struct List {
			Object::Handle base;
		};

		namespace Object {
			// TODO: better system?
			// Override generic in "base.h"
			void update(List& list, F32x2 windowDimensions);
		}
	}
}