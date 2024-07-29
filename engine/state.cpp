#include "state.h"

void initialise(State& state) {
	Font::init();

	state.window = Window::create(&state.storage, Window::Options{});
	state.engine = Engine::create(&state.storage, Engine::Options{}, state.window);

	Input::init(state.window);

	state.context = Commands::Context::create(&state.storage, state.engine);

	state.manager = ResourceManager::Static::create(&state.storage);

	state.guiContext = GUI::Visual::Context::submit(&state.storage, state.manager, state.engine, state.window);
	state.editor.background = createPanel(state.guiContext, PanelSpecification{ nullptr, Color::Gray, Color::Gray, 0.01f });

	ResourceManager::Static::allocate(state.manager, state.engine);

	GUI::Visual::Context::setup(state.guiContext, state.manager, state.context, state.engine);
	properties(state.editor.background)->dimensions = F32x2(0.8f, 0.8f);

	ResourceManager::Static::clearReferences(state.manager);
}

void gameloop(State& state) {
	while (Window::isOpen(state.window, state.engine)) {
		Engine::beginFrame(state.engine, state.window);
		Commands::Context::flush(state.context, state.engine);

		Input::update(state.window);

		Engine::beginRender(state.engine, state.window);

		Panel* addr = &state.editor.background;

		GUI::Visual::Context::updateContext(state.guiContext, Window::dimensions(state.window));
		renderPanels({ &addr, 1 }, state.context, state.guiContext, state.window);

		Engine::endRender(state.engine);

		Engine::endFrame(state.engine, state.window);
	}
}

void terminate(State& state) {
	ResourceManager::Static::drop(&state.storage, state.manager, state.engine);
	Commands::Context::drop(&state.storage, state.context, state.engine);
	GUI::Visual::Context::drop(&state.storage, state.guiContext, state.engine);

	Window::drop(&state.storage, state.window, state.engine);
	Engine::drop(&state.storage, state.engine, state.window);

	Font::terminate();
}