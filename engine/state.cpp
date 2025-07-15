#include "state.h"

void _submit(State& state)
{
	_submitEditor(state);
}

void _submitEditor(State& state)
{
	state.editor.background = createPanel(state.guiContext, PanelSpecification{ defaultGUIParent(state.guiContext), colorDarkGray, colorBlack, 0.0f});

	state.editor.inspectorContainer = createContainer(state.guiContext, ContainerSpecification(
		state.editor.background.base,
		ContainerOrdering::VERTICAL,
		OffsetMethod::EXTENT
	));

	state.editor.createComponent = submitEntry(EntrySpecification<ObjectEntry<VulkanComponent>>(
		VulkanComponent::ENTER_COMPONENT,
		std::vector<VulkanComponent>({
			VulkanComponent::BUFFER,
			VulkanComponent::BUFFER_LAYOUT,
			VulkanComponent::SHADER,
			VulkanComponent::DESCRIPTOR_LAYOUT,
			VulkanComponent::DESCRIPTOR_SET,
			VulkanComponent::PIPELINE
		})
	), state.guiContext, state.manager);

	addChild(state.editor.inspectorContainer.base, { &state.editor.createComponent.base, 1 }, state.guiContext);

	state.editor.compileTree = submitButton(state.manager, state.guiContext, ButtonSpecification(
		state.editor.background.base,
		Color(0.6f, 0.0f, 0.6f),
		Color(0.0f, 0.0f, 0.0f)
	));

	for (uint64_t i = 0; i < state.editor.propertyDisplays.size(); i++) {
		state.editor.propertyDisplays[i] = _submitPropertyDisplay(state, nullEntity, &state.registry);
	}

	_submitEntityView(state);
}

void _submitEntityView(State& state)
{
	state.editor.entityView.background = createPanel(state.guiContext, PanelSpecification{ state.editor.background.base, colorDarkGray, colorBlack, 0.01f });
	state.editor.entityView.createButton = submitButton(state.manager, state.guiContext, ButtonSpecification{ state.editor.entityView.background.base, colorDarkGray, colorBlack });
	state.editor.entityView.entityTree = createTreeContainer(state.guiContext, state.editor.entityView.background.base);
	state.editor.entityView.entityTree.enabled = true;
	state.editor.entityView.entityTextBatch = submitTextBatch(state.manager, state.guiContext, TextBatchSpecification{ 256, state.editor.entityView.createButton.textBatch.font });
	state.editor.entityView.heldElement = nullptr;
	state.editor.entityView.heldEntityPtr = nullptr;
	state.editor.entityView.lastClicked = nullEntity;

	for (uint32_t i = 0; i < MAX_CONCURRENT_ENTITY_PANELS; i++) {
		state.editor.entityView.entityPanels.push_back(createPanel(state.guiContext, PanelSpecification(nullGUIParent(), colorDarkGray, colorBlack, 0.01f)));
		addNewChild(state.editor.entityView.entityTree, &state.editor.entityView.entityPanelIndices[i], state.editor.entityView.entityPanels.back().base, state.guiContext);

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
	setupEntry(state.editor.createComponent, state.manager, state.engine, state.context, state.guiContext);
	setupButton(state.editor.compileTree, state.manager);

	properties(state.editor.compileTree, state.guiContext).dimensions = F32x2(0.2f, 0.05f);
	properties(state.editor.compileTree, state.guiContext).anchorY = GUIAnchor::TOP;
	properties(state.editor.compileTree, state.guiContext).centerY = GUIAnchor::TOP;

	properties(state.editor.inspectorContainer.base, state.guiContext).dimensions = F32x2(0.2f, 1.0f);
	properties(state.editor.inspectorContainer.base, state.guiContext).anchorX = GUIAnchor::RIGHT;
	properties(state.editor.inspectorContainer.base, state.guiContext).centerX = GUIAnchor::RIGHT;
	properties(state.editor.inspectorContainer.base, state.guiContext).anchorY = GUIAnchor::TOP;
	properties(state.editor.inspectorContainer.base, state.guiContext).centerY = GUIAnchor::TOP;
	properties(state.editor.createComponent.base, state.guiContext).dimensions = F32x2(1.0f, 0.05f);
	properties(state.editor.createComponent.base, state.guiContext).anchorY = GUIAnchor::TOP;
	properties(state.editor.createComponent.base, state.guiContext).centerY = GUIAnchor::TOP;

	for (PropertyDisplay& display : state.editor.propertyDisplays) {
		_setupPropertyDisplay(state, display);
		properties(display.base, state.guiContext).dimensions = F32x2(1.0f, 0.9f);
		properties(display.base, state.guiContext).anchorY = GUIAnchor::TOP;
		properties(display.base, state.guiContext).centerY = GUIAnchor::TOP;
	}

	_setupEntityView(state);
}

void _setupEntityView(State& state)
{
	setupButton(state.editor.entityView.createButton, state.manager);
	setButtonText(state.editor.entityView.createButton, state.engine, state.context, state.guiContext, "Create entity");

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

	for (uint32_t i = 0; i < MAX_CONCURRENT_ENTITY_PANELS; i++) {
		GUIProperties& props = properties(state.editor.entityView.entityPanels[i], state.guiContext);
		props.dimensions = F32x2(0.9f, 0.05f);
		props.position = F32x2(0.05f, -0.01f);
		props.centerY = GUIAnchor::TOP;
		props.anchorY = GUIAnchor::TOP;

		GUIProperties& textProps = properties(state.editor.entityView.textObjects[i], state.guiContext);
		textProps.dimensions = F32x2(0.95f);
		textProps.centerX = GUIAnchor::LEFT;
		textProps.centerY = GUIAnchor::BOTTOM;
		textProps.anchorX = GUIAnchor::CENTER;
		textProps.anchorY = GUIAnchor::CENTER;

		state.editor.entityView.entityPanelIndices[i] = i;
	}
}

void _drop(State& state)
{
	_dropEditor(state);
}

void _dropEditor(State& state)
{
	for (PropertyDisplay& display : state.editor.propertyDisplays) {
		_dropPropertyDisplay(state, display);
	}

	dropButton(state.editor.compileTree, state.engine, state.guiContext);
	dropEntry(state.editor.createComponent, state.engine, state.guiContext);
	_dropEntityView(state);
}

void _dropEntityView(State& state)
{
	dropButton(state.editor.entityView.createButton, state.engine, state.guiContext);
	dropTextBatch(state.editor.entityView.entityTextBatch, state.engine);
}

void _update(State& state)
{
	// TODO: does not need to be on every update...
	setButtonText(state.editor.entityView.createButton, state.engine, state.context, state.guiContext, "Entity create");

	bool clicked = Input::get(Input::BTN_LEFT).state == Input::RELEASE;
	bool hoverCreateButton = pointInElement(Input::getCursor(), properties(state.editor.entityView.createButton, state.guiContext));

	// VIVIUM_LOG(LogSeverity::DEBUG, "Holding something: {}", state.editor.entityView.heldEntityPtr != nullptr);

	// TODO: only update the selected entity or the entity we last clicked on?
	for (PropertyDisplay& display : state.editor.propertyDisplays) {
		if (display.entity != nullEntity && state.editor.entityView.lastClicked == display.entity) {
			VIVIUM_LOG(LogSeverity::DEBUG, "Updating a property display {}", (int)display.entity);
			_updatePropertyDisplay(state, display);
		}
	}

	// TODO: need some extra code
	//	- reset the entered value if one was entered
	//	- use the value to add a component to the currently
	//	  rendered property display
	updateEntry(state.editor.createComponent, state.guiContext, state.engine, state.context);
	setButtonText(state.editor.compileTree, state.engine, state.context, state.guiContext, "Compile tree");

	if (clicked && pointInElement(Input::getCursor(), properties(state.editor.compileTree, state.guiContext))) {
		VIVIUM_LOG(LogSeverity::DEBUG, "Clicked compile button");

		_compileTree(state);
	}

	if (getValue(state.editor.createComponent) != VulkanComponent::ENTER_COMPONENT) {
		VulkanComponent toCreate = getValue(state.editor.createComponent);

		// Get property display
		if (state.editor.entityView.lastClicked != nullEntity) {
			switch (toCreate) {
			case VulkanComponent::BUFFER:
				state.registry.addComponent<BufferComponent>(state.editor.entityView.lastClicked, BufferComponent{});
				break;
			case VulkanComponent::SHADER:
				state.registry.addComponent<ShaderComponent>(state.editor.entityView.lastClicked, ShaderComponent{});
				break;
			case VulkanComponent::BUFFER_LAYOUT:
				state.registry.addComponent<BufferLayoutComponent>(state.editor.entityView.lastClicked, BufferLayoutComponent{});
				break;
			case VulkanComponent::DESCRIPTOR_LAYOUT:
				state.registry.addComponent<DescriptorLayoutComponent>(state.editor.entityView.lastClicked, DescriptorLayoutComponent{});
				break;
			case VulkanComponent::DESCRIPTOR_SET:
				state.registry.addComponent<DescriptorSetComponent>(state.editor.entityView.lastClicked, DescriptorSetComponent{});
				break;
			case VulkanComponent::PIPELINE:
				state.registry.addComponent<PipelineComponent>(state.editor.entityView.lastClicked, PipelineComponent{});
				break;
			}
		}

		state.editor.createComponent.currentlySelected = VulkanComponent::ENTER_COMPONENT;
	}

	if (clicked) {
		if (hoverCreateButton) {
			Entity newEntity = state.registry.create();
			state.registry.addComponent<ComponentName>(newEntity, ComponentName{ std::format("Entity {}", newEntity & ECS_ENTITY_MASK) });

			state.editor.entityView.entities.push_back(newEntity);
			// Enable the relevant container
			TreeContainer* container = getContainerByPanel(state.editor.entityView.entities.size() - 1, state.editor.entityView.entityTree);

			VIVIUM_ASSERT(container != nullptr, "Couldn't get container for new panel");

			VIVIUM_LOG(LogSeverity::DEBUG, "Enabling panel by id {}", state.editor.entityView.entities.size() - 1);

			container->enabled = true;

			// Set the entity for the property display
			state.editor.propertyDisplays[state.editor.entityView.entities.size() - 1].entity = newEntity;
		}
	}

	if (clicked) {
		TreeContainer* hovered = getContainer(Input::getCursor(), state.editor.entityView.entityTree, state.guiContext);

		VIVIUM_LOG(LogSeverity::DEBUG, "Clicked on some container");

		if (hovered != nullptr && hovered->data != nullptr) {
			int panelIndex = *(int*)hovered->data;

			state.editor.entityView.lastClicked = state.editor.entityView.entities[panelIndex];

			// TODO: function for refreshing/re-creating inspector window
			clearChildren(state.editor.inspectorContainer.base, state.guiContext);
			addChild(state.editor.inspectorContainer.base, { &state.editor.createComponent.base, 1 }, state.guiContext);
			addChild(state.editor.inspectorContainer.base, { &state.editor.propertyDisplays[panelIndex].base, 1 }, state.guiContext);

			VIVIUM_LOG(LogSeverity::DEBUG, "Last clicked: {}", state.editor.entityView.lastClicked);
		}
	}

	state.editor.entityView.heldElement = updateTreeContainer(Input::getCursor(), state.editor.entityView.entityTree, state.editor.entityView.heldElement, state.guiContext);

	// Look for entity of held element
	if (state.editor.entityView.heldElement != nullptr && state.editor.entityView.heldElement->data != nullptr) {
		int panelIndex = *(int*)state.editor.entityView.heldElement->data;
		// We can assume the panel index is also the index for the relevant entity
		state.editor.entityView.heldEntity = state.editor.entityView.entities[panelIndex];
		state.editor.entityView.heldEntityPtr = &state.editor.entityView.heldEntity;
	}
	else {
		state.editor.entityView.heldEntityPtr = nullptr;
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
	
	// TODO: iterate the tree container
	//	mark the relevant panels that are disabled with appropriate colour

	for (int i = 0; i < std::min(MAX_CONCURRENT_ENTITY_PANELS, (int)state.editor.entityView.entities.size()); i++) {
		entityPanels.push_back(&state.editor.entityView.entityPanels[i]);
	}

	submitPanels(entityPanels, state.guiContext);

	Button* buttons[] = { &state.editor.entityView.createButton, &state.editor.compileTree };

	submitButtons(buttons, state.guiContext);

	ObjectEntry<VulkanComponent>* entries[] = { &state.editor.createComponent };
	submitEntries<VulkanComponent>(entries, state.guiContext);

	// TODO: only render the selected entity or the entity we last clicked on?
	for (PropertyDisplay& display : state.editor.propertyDisplays) {
		if (display.entity != nullEntity && state.editor.entityView.lastClicked == display.entity) {
			_renderPropertyDisplay(state, display);
		}
	}

	GUIProperties& props2 = properties(state.editor.createComponent.base, state.guiContext);
	debugRect(
		props2.minExtent,
		props2.maxExtent - props2.minExtent,
		colorCyan,
		state.guiContext
	);

	renderGUI(state.context, state.guiContext, state.window);

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

TreeContainer* getContainerByPanel(int panelIndex, TreeContainer& container)
{
	if (container.data != nullptr) {
		if (*(int*)container.data == panelIndex) {
			return &container;
		}
	}

	for (TreeContainer& child : container.children) {
		TreeContainer* result = getContainerByPanel(panelIndex, child);

		if (result != nullptr) return result;
	}

	return nullptr;
}

PropertyDisplay _submitPropertyDisplay(State& state, Entity entity, Registry* registry)
{
	PropertyDisplay display;

	display.entity = entity;
	display.registry = registry;
	display.container = createContainer(state.guiContext, ContainerSpecification(
		nullGUIParent(),
		ContainerOrdering::VERTICAL,
		OffsetMethod::EXTENT
	));
	display.base = display.container.base;

	display.buffer = submitEntry(EntrySpecification<BufferEntry>(), state.guiContext, state.manager);
	display.shader = submitEntry(EntrySpecification<ShaderEntry>(), state.guiContext, state.manager);
	display.bufferLayout = submitEntry(EntrySpecification<BufferLayoutEntry>(), state.guiContext, state.manager);
	display.descriptorLayout = submitEntry(EntrySpecification<DescriptorLayoutEntry>(), state.guiContext, state.manager);
	display.descriptor = submitEntry(EntrySpecification<DescriptorSetEntry>(&state.registry, &state.editor.entityView.heldEntityPtr), state.guiContext, state.manager);
	display.pipeline = submitEntry(EntrySpecification<PipelineEntry>(&state.registry, &state.editor.entityView.heldEntityPtr), state.guiContext, state.manager);
	
	setAsleep(display.buffer.base, state.guiContext, true);
	setAsleep(display.shader.base, state.guiContext, true);
	setAsleep(display.bufferLayout.base, state.guiContext, true);
	setAsleep(display.descriptorLayout.base, state.guiContext, true);
	setAsleep(display.descriptor.base, state.guiContext, true);
	setAsleep(display.pipeline.base, state.guiContext, true);

	addChild(display.container.base, { &display.buffer.base, 1 }, state.guiContext);
	addChild(display.container.base, { &display.shader.base, 1 }, state.guiContext);
	addChild(display.container.base, { &display.bufferLayout.base, 1 }, state.guiContext);
	addChild(display.container.base, { &display.descriptorLayout.base, 1 }, state.guiContext);
	addChild(display.container.base, { &display.descriptor.base, 1 }, state.guiContext);
	addChild(display.container.base, { &display.pipeline.base, 1 }, state.guiContext);

	return display;
}

void _setupPropertyDisplay(State& state, PropertyDisplay& display)
{
	setupEntry(display.buffer, state.manager, state.engine, state.context, state.guiContext);
	setupEntry(display.shader, state.manager, state.engine, state.context, state.guiContext);
	setupEntry(display.bufferLayout, state.manager, state.engine, state.context, state.guiContext);
	setupEntry(display.descriptorLayout, state.manager, state.engine, state.context, state.guiContext);
	setupEntry(display.descriptor, state.manager, state.engine, state.context, state.guiContext);
	setupEntry(display.pipeline, state.manager, state.engine, state.context, state.guiContext);

	properties(display.buffer, state.guiContext).dimensions = F32x2(1.0f, 0.04f);
	properties(display.shader, state.guiContext).dimensions = F32x2(1.0f, 0.04f);
	properties(display.bufferLayout, state.guiContext).dimensions = F32x2(1.0f, 0.04f);
	properties(display.descriptorLayout, state.guiContext).dimensions = F32x2(1.0f, 0.04f);
	properties(display.descriptor, state.guiContext).dimensions = F32x2(1.0f, 0.04f);
	properties(display.pipeline, state.guiContext).dimensions = F32x2(1.0f, 0.04f);

	properties(display.buffer, state.guiContext).anchorY = GUIAnchor::TOP;
	properties(display.shader, state.guiContext).anchorY = GUIAnchor::TOP;
	properties(display.bufferLayout, state.guiContext).anchorY = GUIAnchor::TOP;
	properties(display.descriptorLayout, state.guiContext).anchorY = GUIAnchor::TOP;
	properties(display.descriptor, state.guiContext).anchorY = GUIAnchor::TOP;
	properties(display.pipeline, state.guiContext).anchorY = GUIAnchor::TOP;

	properties(display.buffer, state.guiContext).centerY = GUIAnchor::TOP;
	properties(display.shader, state.guiContext).centerY = GUIAnchor::TOP;
	properties(display.bufferLayout, state.guiContext).centerY = GUIAnchor::TOP;
	properties(display.descriptorLayout, state.guiContext).centerY = GUIAnchor::TOP;
	properties(display.descriptor, state.guiContext).centerY = GUIAnchor::TOP;
	properties(display.pipeline, state.guiContext).centerY = GUIAnchor::TOP;
}

void _updatePropertyDisplay(State& state, PropertyDisplay& display)
{
	updateEntry(display.buffer, state.guiContext, state.engine, state.context);
	updateEntry(display.shader, state.guiContext, state.engine, state.context);
	updateEntry(display.bufferLayout, state.guiContext, state.engine, state.context);
	updateEntry(display.descriptorLayout, state.guiContext, state.engine, state.context);
	updateEntry(display.descriptor, state.guiContext, state.engine, state.context);
	updateEntry(display.pipeline, state.guiContext, state.engine, state.context);

	// Figure out which are enabled
	if (display.registry->hasComponent<BufferComponent>(display.entity)) {
		setAsleep(display.buffer.base, state.guiContext, false);
	}

	if (display.registry->hasComponent<ShaderComponent>(display.entity)) {
		setAsleep(display.shader.base, state.guiContext, false);
	}

	if (display.registry->hasComponent<BufferLayoutComponent>(display.entity)) {
		setAsleep(display.bufferLayout.base, state.guiContext, false);
	}

	if (display.registry->hasComponent<DescriptorLayoutComponent>(display.entity)) {
		setAsleep(display.descriptorLayout.base, state.guiContext, false);
	}

	if (display.registry->hasComponent<DescriptorSetComponent>(display.entity)) {
		setAsleep(display.descriptor.base, state.guiContext, false);
	}

	if (display.registry->hasComponent<PipelineComponent>(display.entity)) {
		setAsleep(display.pipeline.base, state.guiContext, false);
	}
}

void _renderPropertyDisplay(State& state, PropertyDisplay& display)
{
	if (display.registry->hasComponent<BufferComponent>(display.entity)) {
		BufferEntry* entry[] = { &display.buffer };
		submitEntries(entry, state.guiContext);
	}

	if (display.registry->hasComponent<ShaderComponent>(display.entity)) {
		ShaderEntry* entry[] = { &display.shader };
		submitEntries(entry, state.guiContext);
	}

	if (display.registry->hasComponent<BufferLayoutComponent>(display.entity)) {
		BufferLayoutEntry* entry[] = { &display.bufferLayout };
		submitEntries(entry, state.guiContext);
	}

	if (display.registry->hasComponent<DescriptorLayoutComponent>(display.entity)) {
		DescriptorLayoutEntry* entry[] = { &display.descriptorLayout };
		submitEntries(entry, state.guiContext);
	}

	if (display.registry->hasComponent<DescriptorSetComponent>(display.entity)) {
		DescriptorSetEntry* entry[] = { &display.descriptor };
		submitEntries(entry, state.guiContext);
	}

	if (display.registry->hasComponent<PipelineComponent>(display.entity)) {
		PipelineEntry* entry[] = { &display.pipeline };
		submitEntries(entry, state.guiContext);
	}
}

void _dropPropertyDisplay(State& state, PropertyDisplay& display)
{
	dropEntry(display.buffer, state.engine, state.guiContext);
	dropEntry(display.shader, state.engine, state.guiContext);
	dropEntry(display.bufferLayout, state.engine, state.guiContext);
	dropEntry(display.descriptorLayout, state.engine, state.guiContext);
	dropEntry(display.descriptor, state.engine, state.guiContext);
	dropEntry(display.pipeline, state.engine, state.guiContext);
}

void _compileTree(State& state)
{
	// 1. loop registry
	// 2. into arrays, sort entities with BufferComponents, ShaderComponents, etc.
	// 3. getValue and update the component of each entity in order
	// 4. once we finally reach the PipelineComponent, we begin serialising everything

	/*std::vector<Entity> bufferComponents;
	std::vector<Entity> shaderComponents;
	std::vector<Entity> bufferLayoutComponents;
	std::vector<Entity> descriptorLayoutComponents;
	std::vector<Entity> descriptorSetComponents;
	std::vector<Entity> pipelineComponents;

	for (uint64_t i = 0; i < state.editor.entityView.entities.size(); i++) {
		Entity entity = state.editor.entityView.entities[i];

		if (state.registry.hasComponent<BufferComponent>(entity)) {
			bufferComponents.push_back(entity);
		}

		if (state.registry.hasComponent<ShaderComponent>(entity)) {
			shaderComponents.push_back(entity);
		}

		if (state.registry.hasComponent<BufferLayoutComponent>(entity)) {
			bufferLayoutComponents.push_back(entity);
		}

		if (state.registry.hasComponent<DescriptorLayoutComponent>(entity)) {
			descriptorLayoutComponents.push_back(entity);
		}

		if (state.registry.hasComponent<DescriptorSetComponent>(entity)) {
			descriptorSetComponents.push_back(entity);
		}

		if (state.registry.hasComponent<PipelineComponent>(entity)) {
			pipelineComponents.push_back(entity);
		}
	}*/

	// TODO: We somewhat want a reverse index from entity -> property display
	//	but this doesn't exactly exist

	// For each display, we want to compile all buffer, shader, buffer layout, and descriptor layout components first
	for (uint64_t i = 0; i < state.editor.propertyDisplays.size(); i++) {
		PropertyDisplay& display = state.editor.propertyDisplays[i];
		Entity entity = display.entity;

		if (state.registry.hasComponent<BufferComponent>(entity)) {
			state.registry.updateComponent<BufferComponent>(entity, getValue(display.buffer));
		}

		if (state.registry.hasComponent<ShaderComponent>(entity)) {
			state.registry.updateComponent<ShaderComponent>(entity, getValue(display.shader));
		}

		if (state.registry.hasComponent<BufferLayoutComponent>(entity)) {
			state.registry.updateComponent<BufferLayoutComponent>(entity, getValue(display.bufferLayout));
		}

		if (state.registry.hasComponent<DescriptorLayoutComponent>(entity)) {
			state.registry.updateComponent<DescriptorLayoutComponent>(entity, getValue(display.descriptorLayout));
		}
	}

	// Then descriptor sets
	for (uint64_t i = 0; i < state.editor.propertyDisplays.size(); i++) {
		PropertyDisplay& display = state.editor.propertyDisplays[i];
		Entity entity = display.entity;

		if (state.registry.hasComponent<DescriptorSetComponent>(entity)) {
			state.registry.updateComponent<DescriptorSetComponent>(entity, getValue(display.descriptor));
		}
	}

	std::vector<PipelineComponent> pipelineComponents;

	// Then pipelines
	for (uint64_t i = 0; i < state.editor.propertyDisplays.size(); i++) {
		PropertyDisplay& display = state.editor.propertyDisplays[i];
		Entity entity = display.entity;

		if (state.registry.hasComponent<PipelineComponent>(entity)) {
			state.registry.updateComponent<PipelineComponent>(entity, getValue(display.pipeline));
			pipelineComponents.push_back(state.registry.getComponent<PipelineComponent>(entity));
		}
	}

	// All values up to date, now grab every entity with a pipeline component and serialise it
	// TODO: bad, but easier than looping entities again
	/*
	BufferLayoutComponent bufferLayout;
	DescriptorLayoutComponent descriptorLayout;
	ShaderComponent vertexShader;
	ShaderComponent fragmentShader;

	BufferComponent vertexBuffer;
	BufferComponent indexBuffer;
	DescriptorSetComponent descriptorSet;
	*/

	// Create serialiser for pipeline
	//		when attempting to read the data to create the relevant objects
	//		we should ideally present it in some dependency order
	//		we also need to be able to reference previous resources we created
	//		can hijack the entity id, but would require some restructuring of the previous step
	//		some indexes should be enough?
	// Horrible counter
	uint32_t objectReferenceIndex = 0;
	SerialiserFileInterface fileInterface;
	fileInterface.begin("vivium4/res/gen.dat", false);

	for (PipelineComponent& pipeline : pipelineComponents) {
		writeComponent(pipeline, fileInterface, objectReferenceIndex);
	}

	fileInterface.end();

	VIVIUM_LOG(LogSeverity::DEBUG, "Wrote {} objects to gen.dat", objectReferenceIndex);
}