#include "state.h"

void _submit(State& state)
{
	_submitEditor(state);
}

void _submitEditor(State& state)
{
	state.editor.background = createPanel(state.guiContext, PanelSpecification{ defaultGUIParent(state.guiContext), colorDarkGray, colorBlack, 0.0f});
	state.editor.testSprite0 = createSprite(state.guiContext, SpriteSpecification(defaultGUIParent(state.guiContext),
		state.editor.entityView.img0.translation, state.editor.entityView.img0.scale));
	state.editor.testSprite1 = createSprite(state.guiContext, SpriteSpecification(defaultGUIParent(state.guiContext),
		state.editor.entityView.img1.translation, state.editor.entityView.img1.scale));

	_submitEntityView(state);
}

void _submitEntityView(State& state)
{
	state.editor.entityView.background = createPanel(state.guiContext, PanelSpecification{ state.editor.background.base, colorDarkGray, colorBlack, 0.01f });
	state.editor.entityView.createButton = submitButton(state.manager, state.guiContext, state.engine, state.window, ButtonSpecification{ state.editor.entityView.background.base, colorDarkGray, colorBlack });
	state.editor.entityView.entityTree = createTreeContainer(state.guiContext, state.editor.entityView.background.base);
	state.editor.entityView.entityTextBatch = submitTextBatch(state.manager, state.engine, state.guiContext, TextBatchSpecification{ 256, state.editor.entityView.createButton.textBatch.font });

	for (uint32_t i = 0; i < MAX_CONCURRENT_ENTITY_PANELS; i++) {
		state.editor.entityView.entityPanels.push_back(createPanel(state.guiContext, PanelSpecification(state.editor.entityView.entityTree.root.base, colorDarkGray, colorBlack, 0.01f)));

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

	properties(state.editor.background.base, state.guiContext).dimensions = F32x2(1.0f);
}

void _setupEntityView(State& state)
{
	setupButton(state.editor.entityView.createButton, state.manager);
	setButtonText(state.editor.entityView.createButton, state.engine, state.window, state.context, state.guiContext, "Create entity");

	setupTextBatch(state.editor.entityView.entityTextBatch, state.manager);

	properties(state.editor.entityView.createButton, state.guiContext).dimensions = F32x2(0.9f, 0.1f);
	properties(state.editor.entityView.createButton, state.guiContext).position = F32x2(0.0f, -0.01f);
	properties(state.editor.entityView.createButton, state.guiContext).centerY = GUIAnchor::TOP;
	properties(state.editor.entityView.createButton, state.guiContext).anchorY = GUIAnchor::TOP;
	properties(state.editor.entityView.background, state.guiContext).dimensions = F32x2(0.2f, 0.9f);
	properties(state.editor.entityView.background, state.guiContext).position = F32x2(0.05f, 0.0f);
	properties(state.editor.entityView.background, state.guiContext).centerX = GUIAnchor::LEFT;
	properties(state.editor.entityView.background, state.guiContext).anchorX = GUIAnchor::LEFT;
	properties(state.editor.entityView.entityTree.root.base, state.guiContext).centerY = GUIAnchor::TOP;
	properties(state.editor.entityView.entityTree.root.base, state.guiContext).anchorY = GUIAnchor::TOP;
	properties(state.editor.entityView.entityTree.root.base, state.guiContext).centerX = GUIAnchor::CENTER;
	properties(state.editor.entityView.entityTree.root.base, state.guiContext).anchorX = GUIAnchor::CENTER;
	properties(state.editor.entityView.entityTree.root.base, state.guiContext).position = F32x2(0.0f, -0.115f);
	properties(state.editor.entityView.entityTree.root.base, state.guiContext).dimensions = F32x2(1.0f, 1.0f);
	properties(state.editor.testSprite0, state.guiContext).dimensions = F32x2(0.2f, 0.2f);
	properties(state.editor.testSprite1, state.guiContext).dimensions = F32x2(0.2f, 0.2f);
	properties(state.editor.testSprite1, state.guiContext).position = F32x2(0.2f, 0.2f);

	for (uint32_t i = 0; i < MAX_CONCURRENT_ENTITY_PANELS; i++) {
		GUIProperties& props = properties(state.editor.entityView.entityPanels[i], state.guiContext);
		props.dimensions = F32x2(0.9f, 0.05f);
		props.position = F32x2(0.0f, -0.01f);
		props.centerY = GUIAnchor::TOP;
		props.anchorY = GUIAnchor::TOP;

		GUIProperties& textProps = properties(state.editor.entityView.textObjects[i], state.guiContext);
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
}

void _dropEntityView(State& state)
{
	dropButton(state.editor.entityView.createButton, state.engine, state.guiContext);
	dropTextBatch(state.editor.entityView.entityTextBatch, state.engine);
}

void _update(State& state)
{
	setButtonText(state.editor.entityView.createButton, state.engine, state.window, state.context, state.guiContext, "Entity create");

	if (pointInElement(Input::getCursor(), properties(state.editor.entityView.createButton, state.guiContext)) && Input::get(Input::BTN_LEFT).state == Input::PRESS) {
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

	calculateTextBatch(state.editor.entityView.entityTextBatch, textObjectsPtr, state.context, state.guiContext, state.engine);
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

	std::vector<Sprite*> sprites;
	sprites.push_back(&state.editor.testSprite0);
	sprites.push_back(&state.editor.testSprite1);
	renderSprites(sprites, state.context, state.guiContext, state.window);

	Button* buttons[] = { &state.editor.entityView.createButton };
	renderButtons(buttons, state.context, state.guiContext, state.window);

	renderTextBatch(state.editor.entityView.entityTextBatch, state.context, state.guiContext, orthogonalPerspective2D(windowDimensions(state.window), F32x2(0.0f), 0.0f, 1.0f));
}

StitchedAtlas _createSpriteAtlas(State& state)
{
	StitchedAtlasCreator creator = createStitchedAtlasCreator(TextureFormat::RGBA);
	StitchedAtlasReference img0 = submitToStitchedAtlasCreator("vivium4/res/img0.png", creator);
	StitchedAtlasReference img1 = submitToStitchedAtlasCreator("vivium4/res/img1.png", creator);
	StitchedAtlas atlas = finishAtlasCreation(creator);
	dropAtlasCreator(creator);
	state.editor.entityView.img0 = convertStitchedAtlasReference(img0, atlas);
	state.editor.entityView.img1 = convertStitchedAtlasReference(img1, atlas);

	return atlas;
}

void initialise(State& state)
{
	_logInit(); // TODO: ugly that we have to initialise this
	_fontInit();

	state.engine = createEngine(EngineOptions{});
	state.window = createWindow(WindowOptions{}, state.engine);

	Input::init(state.window);

	state.context = createCommandContext(state.engine);

	state.manager = createManager();
	
	StitchedAtlas atlas = _createSpriteAtlas(state);

	state.guiContext = createGUIContext(state.manager, state.engine, state.window, &atlas);

	_submit(state);

	allocateManager(state.manager, state.engine);

	setupGUIContext(state.guiContext, state.manager, state.context, state.engine);
	_setup(state);

	clearManagerReferences(state.manager);
	dropAtlas(atlas);
}

void gameloop(State& state) {
	while (windowIsOpen(state.window, state.engine)) {
		engineBeginFrame(state.engine, state.context);

		Input::update(state.window);

		updateGUI(windowDimensions(state.window), state.guiContext);
		_update(state);

		windowBeginFrame(state.window, state.context, state.engine);
		windowBeginRender(state.window);

		_draw(state);
		
		windowEndRender(state.window);
		windowEndFrame(state.window, state.engine);

		engineEndFrame(state.engine);
	}
}

void terminate(State& state) {
	dropManager(state.manager, state.engine);
	dropCommandContext(state.context, state.engine);
	dropGUIContext(state.guiContext, state.engine);

	_drop(state);

	dropWindow(state.window, state.engine);
	dropEngine(state.engine);

	_fontTerminate();
}