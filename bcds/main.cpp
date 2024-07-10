#pragma once

#include "../vivium4/vivium4.h"

using namespace Vivium;

struct State {
	Engine::Handle engine;
	Window::Handle window;
	Commands::Context::Handle context;
	GUI::Visual::Context::Handle guiContext;
	ResourceManager::Static::Handle manager;

	Storage::Static::Pool storage;

	Math::Perspective perspective;

	struct {
		Button button;
	} choiceMenu;
};

void initialise(State& state) {
	Font::init();

	bool regenFont = false;

	// TODO: move to vivium text rendering
	if (!std::filesystem::exists("res/fonts/consola.sdf") || regenFont)
	{
		Font::compileSignedDistanceField("res/fonts/consola.ttf", 512, "res/fonts/consola.sdf", 48, 1.0f);
	}

	state.window = Window::create(&state.storage, Window::Options{});
	state.engine = Engine::create(&state.storage, Engine::Options{}, state.window);

	Input::init(state.window);

	state.context = Commands::Context::create(&state.storage, state.engine);

	state.manager = ResourceManager::Static::create(&state.storage);

	state.guiContext = GUI::Visual::Context::submit(&state.storage, state.manager, state.engine, state.window);
	state.choiceMenu.button = submitButton(state.manager, state.guiContext, state.engine, state.window);

	ResourceManager::Static::allocate(state.manager, state.engine);

	GUI::Visual::Context::setup(state.guiContext, state.manager, state.context, state.engine);
	setupButton(state.choiceMenu.button, state.manager);
	setButtonText(state.choiceMenu.button, state.engine, state.window, state.context, "Hello");
	state.choiceMenu.button.base->properties.dimensions = F32x2(0.8f, 0.8f);

	ResourceManager::Static::clearReferences(state.manager);
}

void gameloop(State& state) {
	while (Window::isOpen(state.window, state.engine)) {
		Engine::beginFrame(state.engine, state.window);
		Commands::Context::flush(state.context, state.engine);

		Input::update(state.window);

		Engine::beginRender(state.engine, state.window);

		Button* addr = &state.choiceMenu.button;

		renderButtons({ &addr, 1 }, state.context, state.guiContext, state.window);

		Engine::endRender(state.engine);

		Engine::endFrame(state.engine, state.window);
	}
}

void terminate(State& state) {
	ResourceManager::Static::drop(&state.storage, state.manager, state.engine);
	Commands::Context::drop(&state.storage, state.context, state.engine);
	GUI::Visual::Context::drop(&state.storage, state.guiContext, state.engine);

	dropButton(state.choiceMenu.button, state.engine);

	Window::drop(&state.storage, state.window, state.engine);
	Engine::drop(&state.storage, state.engine, state.window);

	Font::terminate();
}

int main(void) {
	State state;

	initialise(state);
	gameloop(state);
	terminate(state);

	return NULL;
}