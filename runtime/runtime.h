#pragma once

#include "../engine/state.h"
#include "../vivium4/vivium4.h"

using namespace Vivium;

// TODO: use these?
constexpr char const* SCRIPT_FUNCTION_NAME_SUBMIT = "vSubmit";
constexpr char const* SCRIPT_FUNCTION_NAME_SETUP = "vSetup";
constexpr char const* SCRIPT_FUNCTION_NAME_UPDATE = "vUpdate";
constexpr char const* SCRIPT_FUNCTION_NAME_DRAW = "vDraw";
constexpr char const* SCRIPT_FUNCTION_NAME_DROP = "vDrop";

enum LuaDataType { UINT8, UINT16, UINT32, FLOAT, VEC2, VEC3 };

namespace Runtime {
// TODO: terrible names on all of these
struct DescriptorSetObjects {
  UniformType type;

  Ref<Buffer> buffer;
  Ref<Framebuffer> framebuffer;  // TODO
  Ref<Texture> texture;          // TODO
  Entity entity;

  BufferComponent bufferComponent;
};

// TODO: only supports one type of draw command
struct PipelineInstance {
  Ref<Pipeline> pipeline;
  Ref<Buffer> indexBuffer;
  Ref<Buffer> vertexBuffer;
  Ref<Shader> vertexShader;
  Ref<Shader> fragmentShader;
  Ref<DescriptorSet> descriptor;
  Ref<DescriptorLayout> layout;

  // Store the entity we're on
  Entity entity;
  uint16_t indexCount;
  uint64_t instanceCount;

  // TODO: in future we need to upload data to the descriptor set
  // buffers/framebuffers?
  //	we also need to know what data to upload to them, so we need the
  // permanent component reference 	and alongside that component reference, the
  // buffer itself
  // also need permanent reference for any buffer/framebuffer/texture for drop
  std::vector<DescriptorSetObjects> descriptorObjects;

  PipelineComponent component;
};

struct ScriptMetadata {
  std::string name;

  int submitRef;
  int setupRef;
  int updateRef;
  int drawRef;
  int dropRef;
};

enum Stage {
  SUBMIT = 0x1,
  SETUP = 0x2,
  UPDATE = 0x4,
  DRAW = 0x8,
  DROP = 0x10,
};

struct LuaContext {
  Registry* registry;
  std::unordered_map<Entity, Entity>* entityMap;
  std::vector<PipelineInstance>* pipelineInstances;
  // TODO: store state (submit/setup/etc.) in here so we can check function
  // usage is valid
  Stage stage;
};

struct State {
  Engine engine;
  Window window;
  CommandContext context;
  ResourceManager manager;

  std::vector<ScriptMetadata> scripts;

  SerialiserFileInterface store;
  Registry registry;

  View<Owned<PipelineComponent>> pipelineComponents;
  std::vector<PipelineInstance> pipelineInstances;
  // TODO: might not be necessary with some smart creation of registry
  //	or a better registry serialisation method
  // When an entity is loaded from serialised data, we map the loaded entity ID
  //	to the new runtime entity
  // If we request an entity, we thus return the runtime entity from this map
  //	and any set/update component requests just directly update that
  // component
  std::unordered_map<Entity, Entity> entityMap;

  lua_State* L;
  LuaContext luaContext;
};

void _submit(State& state);
void _setup(State& state);
void _update(State& state);
void _draw(State& state);

void _loadRegistry(State& state);
PipelineInstance _pipelineInstanceFromComponent(
    State& state, PipelineComponent const& component, Entity entity);

void _loadScripts(State& state);
ScriptMetadata _loadScript(State& state, std::string path);
void _runScriptFunction(State& state, int functionIndex);
void _clearScriptFunctionReference(State& state, int functionIndex);

void init(State& state, std::string bytecodeFilename);
void run(State& state);
void drop(State& state);

void _pushLuaFunction(State& state, lua_CFunction function,
                      std::string_view name);
void _loadLuaObjects(State& state);
void _loadLuaEntity(State& state);
void _loadLuaComponentsEnum(State& state);
void _loadLuaDataTypesEnum(State& state);

int _luaBlock(lua_State* L);

// TODO: move to the other constexpr at top of file
inline constexpr char const* ENTITY_TABLE_NAME = "EntityMeta";

int _luaCreateEntity(lua_State* L);
int _luaEntityDrop(lua_State* L);
int _luaEntityString(lua_State* L);
int _luaEntityID(lua_State* L);
int _luaGetComponent(lua_State* L);
int _luaSetBufferData(lua_State* L);
int _luaDrawIndex(lua_State* L);
int _luaCursorPosition(lua_State* L);
int _luaIsLeftClick(lua_State* L);
int _luaIsRightClick(lua_State* L);

}  // namespace Runtime
