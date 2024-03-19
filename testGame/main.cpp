#include "../vivium4/vivium4.h"

using namespace Vivium;

int main(void) {
	Allocator::Linear storage;
	Engine::Handle engine = Engine::create(storage, Engine::Options{});
	Window::Handle window = Window::create(window, Window::Options{});

	Engine::setWindow(engine, window);

	while (Window::isOpen(window)) {
		Engine::beginFrame();
		Engine::endFrame();
	}

	Window::close(storage, window);
	Engine::close(storage, engine);
}