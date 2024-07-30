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

		struct {
			Panel background;
			Button createButton;

			std::vector<Entity> entities;
		} entityView;
	} editor;
};

inline constexpr Color colorBlack =		Color(0.0f, 0.0f, 0.0f);
inline constexpr Color colorDarkGray =	Color(0.3f, 0.3f, 0.3f);
inline constexpr Color colorGray =		Color(0.5f, 0.5f, 0.5f);
inline constexpr Color colorWhite =		Color(1.0f, 1.0f, 1.0f);
inline constexpr Color colorCyan =		Color(0.1f, 0.5f, 0.8f);

void _submit(State& state);
void _submitEditor(State& state);
void _submitEntityView(State& state);

void _setup(State& state);
void _setupEditor(State& state);
void _setupEntityView(State& state);

void _drop(State& state);
void _dropEditor(State& state);
void _dropEntityView(State& state);

void _update(State& state);
void _draw(State& state);

void initialise(State& state);
void gameloop(State& state);
void terminate(State& state);