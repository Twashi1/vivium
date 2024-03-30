#include "../vivium4/vivium4.h"

using namespace Vivium;

int main(void) {
	Allocator::Linear storage;
	Window::Handle window = Window::create(window, Window::Options{});
	Engine::Handle engine = Engine::create(storage, Engine::Options{}, window);
	Commands::Context::Handle context = Commands::Context::create(storage, engine);
	
	// TODO: create function for this
	ResourceManager::Static::Handle<decltype(storage)> manager;

	while (Window::isOpen(window, engine)) {
		Engine::beginFrame(engine, window);
		Commands::Context::flush(context, engine);

		Engine::beginRender(engine, window);

		// DO stuff

		Engine::endRender(engine);
		Engine::endFrame(engine, window);
	}

	Window::drop(storage, window);
	Engine::drop(storage, engine, window);
	storage.free();
}