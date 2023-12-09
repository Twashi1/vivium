#include "vivium4.h"

using namespace Vivium;

int main()
{
	Engine::Handle engine = Engine::create(Engine::Options{});
	Window::Handle window = Window::create(Window::Options{});

	while (Window::isOpen(window)) {
		Engine::beginCommand(engine, window);
		Engine::beginPass(engine);
		// Do stuff
		Engine::endPass(engine);
		Engine::endCommand(engine);
	}

	Window::close(window);
	Engine::close(engine);

	return NULL;
}
