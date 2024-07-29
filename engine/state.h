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
		Panel background;
	} editor;
};

void initialise(State& state);
void gameloop(State& state);
void terminate(State& state);