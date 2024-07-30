#include "state.h"

void _submit(State& state)
{
	_submitEditor(state);
}

void _submitEditor(State& state)
{
	state.editor.background = createPanel(state.guiContext, PanelSpecification{ nullptr, Color::Gray, Color::White, 0.0f });

	_submitEntityView(state);
}

void _submitEntityView(State& state)
{
	state.editor.entityView.background = createPanel(state.guiContext, PanelSpecification{ state.editor.background.base, colorDarkGray, colorBlack, 0.01f });
	state.editor.entityView.createButton = submitButton(state.manager, state.guiContext, state.engine, state.window, ButtonSpecification{ state.editor.entityView.background.base, Color::Gray, Color::Black });
}

void _setup(State& state)
{
	_setupEditor(state);
}

void _setupEditor(State& state)
{
	_setupEntityView(state);

	properties(state.editor.background)->dimensions = F32x2(1.0f);
}

void _setupEntityView(State& state)
{
	setupButton(state.editor.entityView.createButton, state.manager);
	setButtonText(state.editor.entityView.createButton, state.engine, state.window, state.context, "Create entity");

	properties(state.editor.entityView.createButton)->dimensions = F32x2(0.9f, 0.1f);
	properties(state.editor.entityView.createButton)->position = F32x2(0.0f, -0.01f);
	properties(state.editor.entityView.createButton)->centerY = GUIAnchor::TOP;
	properties(state.editor.entityView.createButton)->anchorY = GUIAnchor::TOP;
	properties(state.editor.entityView.background)->dimensions = F32x2(0.2f, 0.9f);
	properties(state.editor.entityView.background)->position = F32x2(0.05f, 0.0f);
	properties(state.editor.entityView.background)->centerX = GUIAnchor::LEFT;
	properties(state.editor.entityView.background)->anchorX = GUIAnchor::LEFT;
}

void _drop(State& state)
{
	_dropEditor(state);
}

void _dropEditor(State& state)
{
	_dropEntityView(state);
}

void _dropEntityView(State& state)
{
	dropButton(state.editor.entityView.createButton, state.engine);
}

void _update(State& state)
{
	if (pointInElement(Input::getCursor(), *properties(state.editor.entityView.createButton)) && Input::get(Input::BTN_LEFT).state == Input::PRESS) {
		VIVIUM_LOG(Log::DEBUG, "In button!");
	}
}

void _draw(State& state)
{
	Panel* panels[] = { &state.editor.background, &state.editor.entityView.background };
	renderPanels(panels, state.context, state.guiContext, state.window);

	Button* buttons[] = { &state.editor.entityView.createButton };
	renderButtons(buttons, state.context, state.guiContext, state.window);
}

void initialise(State& state) {
	Font::init();

	state.window = Window::create(&state.storage, Window::Options{});
	state.engine = Engine::create(&state.storage, Engine::Options{}, state.window);

	Input::init(state.window);

	state.context = Commands::Context::create(&state.storage, state.engine);

	state.manager = ResourceManager::Static::create(&state.storage);

	state.guiContext = GUI::Visual::Context::submit(&state.storage, state.manager, state.engine, state.window);

	_submit(state);

	ResourceManager::Static::allocate(state.manager, state.engine);

	GUI::Visual::Context::setup(state.guiContext, state.manager, state.context, state.engine);
	_setup(state);

	ResourceManager::Static::clearReferences(state.manager);
}

void gameloop(State& state) {
	while (Window::isOpen(state.window, state.engine)) {
		Engine::beginFrame(state.engine, state.window);
		Commands::Context::flush(state.context, state.engine);

		Input::update(state.window);

		GUI::Visual::Context::updateContext(state.guiContext, Window::dimensions(state.window));
		_update(state);

		Engine::beginRender(state.engine, state.window);

		_draw(state);

		Engine::endRender(state.engine);

		Engine::endFrame(state.engine, state.window);
	}
}

void terminate(State& state) {
	ResourceManager::Static::drop(&state.storage, state.manager, state.engine);
	Commands::Context::drop(&state.storage, state.context, state.engine);
	GUI::Visual::Context::drop(&state.storage, state.guiContext, state.engine);

	_drop(state);

	Window::drop(&state.storage, state.window, state.engine);
	Engine::drop(&state.storage, state.engine, state.window);

	Font::terminate();
}