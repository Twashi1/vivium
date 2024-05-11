#include "list.h"

namespace Vivium {
	namespace GUI {
		namespace Object {
			void update(List& list, F32x2 windowDimensions) {
				updateHandle(list.base, windowDimensions);

				for (uint64_t i = 1; i < list.base->children.size(); i++) {
					Handle before = list.base->children[i - 1];
					Handle current = list.base->children[i];

					// Look at child before's position
					// Set position to that child's position + our dimensions
					current->properties.truePosition = before->properties.truePosition + current->properties.trueDimensions;
				}
			}
		}
	}
}