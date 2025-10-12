#pragma once

#include "components.h"
#include "engine.h"
#include "tree_container.h"

struct ComponentName {
  std::string name;
};

namespace Vivium {
template <SerialiserInterface Store>
void serialiseWrite(ComponentName const& name, Store& store) {
  dispatchSerialiseWrite(name.name, store);
}

template <SerialiserInterface Store>
void serialiseRead(ComponentName* name, Store& store) {
  dispatchSerialiseRead(&name->name, store);
}
}  // namespace Vivium

inline constexpr int MAX_CONCURRENT_ENTITY_PANELS = 16;

struct PropertyDisplay {
  GUIElementReference base;
  Container container;

  Entity entity;
  Registry* registry;

  PipelineEntry pipeline;
  BufferLayoutEntry bufferLayout;
  DescriptorLayoutEntry descriptorLayout;
  DescriptorSetEntry descriptor;
  BufferEntry buffer;
  TextureEntry texture;
  ShaderEntry shader;
};

// Goal is to serialise the editor
//	so we can reload it
// 1. save entire registry
// 2. save entity tree (Tree container tree)

struct State {
  Engine engine;
  Window window;
  CommandContext context;
  GUIContext guiContext;
  ResourceManager manager;

  Perspective perspective;

  Registry registry;

  struct {
    Panel background;

    // Need to be able to render a variable amount of different entries
    //	we know each entity can have strictly one of each type of component
    //entry
    Container inspectorContainer;
    ObjectEntry<VulkanComponent> createComponent;
    std::array<PropertyDisplay, MAX_CONCURRENT_ENTITY_PANELS> propertyDisplays;

    Button compileTree;
    Button saveProject;
    Button loadProject;

    struct {
      Panel background;
      Button createButton;
      TextBatch entityTextBatch;

      TreeContainer entityTree;
      TreeContainer* heldElement;
      std::array<int, MAX_CONCURRENT_ENTITY_PANELS> entityPanelIndices;

      std::vector<Entity> entities;
      std::vector<Text> textObjects;
      std::vector<Panel> entityPanels;

      Entity* heldEntityPtr;
      Entity heldEntity;
      Entity lastClicked;

      AtlasIndex img0;
      AtlasIndex img1;
    } entityView;
  } editor;
};

inline constexpr Color colorBlack = Color(0.0f, 0.0f, 0.0f);
inline constexpr Color colorDarkGray = Color(0.3f, 0.3f, 0.3f);
inline constexpr Color colorGray = Color(0.5f, 0.5f, 0.5f);
inline constexpr Color colorWhite = Color(1.0f, 1.0f, 1.0f);
inline constexpr Color colorCyan = Color(0.1f, 0.5f, 0.8f);

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

StitchedAtlas _createSpriteAtlas(State& state);

void initialise(State& state);
void gameloop(State& state);
void terminate(State& state);

TreeContainer* getContainerByPanel(int panelIndex, TreeContainer& container);

PropertyDisplay _submitPropertyDisplay(State& state, Entity entity,
                                       Registry* registry);
void _setupPropertyDisplay(State& state, PropertyDisplay& display);
void _updatePropertyDisplay(State& state, PropertyDisplay& display);
void _renderPropertyDisplay(State& state, PropertyDisplay& display);
void _dropPropertyDisplay(State& state, PropertyDisplay& display);

void _addEntityButton(State& state);

void _updateComponentValues(State& state);
void _updateEntryValues(State& state);
void _compileTree(State& state);

void _saveState(State& state, std::string_view filename);
void _loadState(State& state, std::string_view filename);
