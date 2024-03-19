#pragma once

#include "../../window.h"
#include "core.h"

namespace Vivium {
	namespace Panel {
		struct Resource {
			union Parent {
				Handle panel;
				Window::Handle window;
			};

			Parent parent;
			F32x2 position;
		};

		typedef Resource* Handle;
	}
}