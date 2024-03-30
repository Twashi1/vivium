#include "../vivium4/vivium4.h"

using namespace Vivium;

int main(void) {
	Allocator::Static::Pool storage = Allocator::Static::Pool(4096);
	Window::Handle window = Window::create(storage, Window::Options{});
	Engine::Handle engine = Engine::create(storage, Engine::Options{}, window);
	Commands::Context::Handle context = Commands::Context::create(storage, engine);
	
	// TODO: create function for this
	ResourceManager::Static::Handle manager = ResourceManager::Static::create(storage);

	// Submit calls

	// TODO: should be namespace function
	manager->allocate(engine);

	while (Window::isOpen(window, engine)) {
		Engine::beginFrame(engine, window);
		Commands::Context::flush(context, engine);

		Engine::beginRender(engine, window);

		// DO stuff

		Engine::endRender(engine);
		Engine::endFrame(engine, window);
	}

	ResourceManager::Static::drop(storage, engine, manager);

	// TODO: delete order not obvious
	Window::drop(storage, window, engine);
	Engine::drop(storage, engine, window);
	storage.free();
}