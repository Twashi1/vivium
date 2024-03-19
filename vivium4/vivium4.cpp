#include "vivium4.h"

using namespace Vivium;

int main()
{
	Storage::Scoped<Allocator::Linear> scoped;

	Engine::Handle engine = Engine::create(scoped, Engine::Options{});
	Window::Handle window = Window::create(scoped, Window::Options{});

	while (Window::isOpen(window)) {
		Engine::beginCommand(engine, window);
		Engine::beginPass(engine);
		// Do stuff
		Engine::endPass(engine);
		Engine::endCommand(engine);
	}

	Window::close(scoped, window);
	Engine::close(scoped, engine);

	return NULL;
}
