#pragma once

#include "../vivium4/vivium4.h"

using namespace Vivium;

struct ComponentName {
	std::string name;
};

struct ComponentPosition {
	F32x2 position;
};

inline constexpr int MAX_CONCURRENT_ENTITY_PANELS = 32;

struct State {
	Engine::Handle engine;
	Window::Handle window;
	Commands::Context::Handle context;
	GUIContext guiContext;
	ResourceManager manager;

	Storage::Static::Pool storage;

	Math::Perspective perspective;

	Registry registry;

	struct {
		Panel background;

		struct {
			Panel background;
			Button createButton;
			TextBatch entityTextBatch;

			GUIElementReference entityObjectsElement;

			std::vector<Entity> entities;
			std::vector<Text> textObjects;
			std::vector<Panel> entityPanels;
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