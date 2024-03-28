#include "../vivium4/vivium4.h"

using namespace Vivium;

int main(void) {
	Allocator::Linear storage;
	Window::Handle window = Window::create(window, Window::Options{});
	Engine::Handle engine = Engine::create(storage, Engine::Options{}, window);

	while (Window::isOpen(window, engine)) {
		Engine::beginFrame(engine, window);
		Engine::beginRender(engine, window);
		Engine::endRender(engine);
		Engine::endFrame(engine, window);
	}

	Window::close(storage, window);
	Engine::close(storage, engine, window);
	storage.free();
}