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
	// TODO: VERY TEMPORARY
	state.editor.entityView.entityObjectsElement = new GUIElement;

	state.editor.entityView.background = createPanel(state.guiContext, PanelSpecification{ state.editor.background.base, colorDarkGray, colorBlack, 0.01f });
	state.editor.entityView.createButton = submitButton(state.manager, state.guiContext, state.engine, state.window, ButtonSpecification{ state.editor.entityView.background.base, Color::Gray, Color::Black });
	state.editor.entityView.entityTextBatch = submitTextBatch(state.manager, state.engine, state.guiContext, TextBatchSpecification{ 256, state.editor.entityView.createButton.textBatch.font });

	for (uint32_t i = 0; i < MAX_CONCURRENT_ENTITY_PANELS; i++) {
		state.editor.entityView.entityPanels.push_back(createPanel(state.guiContext, PanelSpecification(state.editor.entityView.entityObjectsElement, colorDarkGray, colorBlack, 0.01f)));
		state.editor.entityView.textObjects.push_back(createText(TextSpecification{
			state.editor.entityView.entityPanels.back().base,
			"",
			colorCyan,
			calculateTextMetrics("", state.editor.entityView.entityTextBatch.font),
			TextAlignment::CENTER
			}, state.guiContext));
	}
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

	setupTextBatch(state.editor.entityView.entityTextBatch, state.manager);

	addChild(state.editor.entityView.background, state.editor.entityView.entityObjectsElement);

	properties(state.editor.entityView.createButton)->dimensions = F32x2(0.9f, 0.1f);
	properties(state.editor.entityView.createButton)->position = F32x2(0.0f, -0.01f);
	properties(state.editor.entityView.createButton)->centerY = GUIAnchor::TOP;
	properties(state.editor.entityView.createButton)->anchorY = GUIAnchor::TOP;
	properties(state.editor.entityView.background)->dimensions = F32x2(0.2f, 0.9f);
	properties(state.editor.entityView.background)->position = F32x2(0.05f, 0.0f);
	properties(state.editor.entityView.background)->centerX = GUIAnchor::LEFT;
	properties(state.editor.entityView.background)->anchorX = GUIAnchor::LEFT;
	properties(state.editor.entityView.entityObjectsElement)->centerY = GUIAnchor::TOP;
	properties(state.editor.entityView.entityObjectsElement)->anchorY = GUIAnchor::TOP;
	properties(state.editor.entityView.entityObjectsElement)->position = F32x2(0.0f, -0.115f);

	for (uint32_t i = 0; i < MAX_CONCURRENT_ENTITY_PANELS; i++) {
		GUIProperties& props = *properties(state.editor.entityView.entityPanels[i]);
		
		props.dimensions = F32x2(0.9f, 0.05f);
		props.position = F32x2(0.0f, -0.015f + -0.075f * i);
		props.centerY = GUIAnchor::TOP;
		props.anchorY = GUIAnchor::TOP;

		GUIProperties& textProps = *properties(state.editor.entityView.textObjects[i]);
		textProps.dimensions = F32x2(0.95f);
		textProps.centerX = GUIAnchor::LEFT;
		textProps.centerY = GUIAnchor::BOTTOM;
		textProps.anchorX = GUIAnchor::CENTER;
		textProps.anchorY = GUIAnchor::CENTER;
	}
}

void _drop(State& state)
{
	_dropEditor(state);
}

void _dropEditor(State& state)
{
	_dropEntityView(state);

	dropPanel(state.editor.background, state.guiContext);
}

void _dropEntityView(State& state)
{
	// TODO: VERY TEMPORARY
	delete state.editor.entityView.entityObjectsElement;

	dropButton(state.editor.entityView.createButton, state.engine, state.guiContext);
	dropPanel(state.editor.entityView.background, state.guiContext);
	dropTextBatch(state.editor.entityView.entityTextBatch, state.engine);

	for (Text& text : state.editor.entityView.textObjects) {
		dropText(text, state.guiContext);
	}

	for (Panel& panel : state.editor.entityView.entityPanels) {
		dropPanel(panel, state.guiContext);
	}
}

void _update(State& state)
{
	setButtonText(state.editor.entityView.createButton, state.engine, state.window, state.context, "Entity create");

	if (pointInElement(Input::getCursor(), *properties(state.editor.entityView.createButton)) && Input::get(Input::BTN_LEFT).state == Input::PRESS) {
		Entity newEntity = state.registry.create();
		state.registry.addComponent<ComponentName>(newEntity, ComponentName{ std::format("Entity {}", newEntity & ECS_ENTITY_MASK) });

		state.editor.entityView.entities.push_back(newEntity);
	}

	// TODO: get view from registry
	int i = 0;
	std::vector<Text*> textObjectsPtr;

	for (Entity e : state.editor.entityView.entities) {
		ComponentName& name = state.registry.getComponent<ComponentName>(e);

		textObjectsPtr.push_back(&state.editor.entityView.textObjects[i]);
		state.editor.entityView.textObjects[i].metrics = calculateTextMetrics(name.name, state.editor.entityView.entityTextBatch.font);
		state.editor.entityView.textObjects[i++].characters = name.name;
	}

	calculateTextBatch(state.editor.entityView.entityTextBatch, textObjectsPtr, state.context, state.engine);
}

void _draw(State& state)
{
	std::vector<Panel*> entityPanels;

	entityPanels.push_back(&state.editor.background);
	entityPanels.push_back(&state.editor.entityView.background);

	for (int i = 0; i < std::min(MAX_CONCURRENT_ENTITY_PANELS, (int)state.editor.entityView.entities.size()); i++) {
		entityPanels.push_back(&state.editor.entityView.entityPanels[i]);
	}
	renderPanels(entityPanels, state.context, state.guiContext, state.window);

	Button* buttons[] = { &state.editor.entityView.createButton };
	renderButtons(buttons, state.context, state.guiContext, state.window);

	renderTextBatch(state.editor.entityView.entityTextBatch, state.context, state.guiContext, Math::orthogonalPerspective2D(state.window, F32x2(0.0f), 0.0f, 1.0f));
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