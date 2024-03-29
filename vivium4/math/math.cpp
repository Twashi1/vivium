#include "math.h"

namespace Vivium {
	namespace Math {
		Perspective calculatePerspective(Window::Handle window, F32x2 position, float rotation, float scale)
		{
			I32x2 screen_dim = Window::dimensions(window);

			Perspective perspective;

			perspective.projection = glm::ortho(0.0f, static_cast<float>(screen_dim.x), static_cast<float>(screen_dim.y), 0.0f, -1.0f, 1.0f);
			glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(scale))
				* glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1))
				* glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f));
			perspective.view = glm::inverse(transform);
		}
	}
}