#pragma once

#include "../vivium4/vivium4.h"
#include "../engine/state.h"

using namespace Vivium;

constexpr char const* SCRIPT_FUNCTION_NAME_SUBMIT = "vSubmit";
constexpr char const* SCRIPT_FUNCTION_NAME_SETUP = "vSetup";
constexpr char const* SCRIPT_FUNCTION_NAME_UPDATE = "vUpdate";
constexpr char const* SCRIPT_FUNCTION_NAME_DRAW = "vDraw";
constexpr char const* SCRIPT_FUNCTION_NAME_DROP = "vDrop";

namespace Runtime {
	// TODO: terrible names on all of these
	struct DescriptorSetObjects {
		UniformType type;

		Ref<Buffer> buffer;
		Ref<Framebuffer> framebuffer;  // TODO
		Ref<Texture> texture;		   // TODO

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

		uint16_t indexCount;

		// TODO: in future we need to upload data to the descriptor set buffers/framebuffers?
		//	we also need to know what data to upload to them, so we need the permanent component reference
		//	and alongside that component reference, the buffer itself
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
		//	and any set/update component requests just directly update that component
		std::unordered_map<Entity, Entity> entityMap;

		lua_State* L;
	};

	void _submit(State& state);
	void _setup(State& state);
	void _update(State& state);
	void _draw(State& state);

	void _loadRegistry(State& state);
	PipelineInstance _pipelineInstanceFromComponent(State& state, PipelineComponent const& component);

	void _loadScripts(State& state);
	ScriptMetadata _loadScript(State& state, std::string path);
	void _runScriptFunction(State& state, int functionIndex);
	void _clearScriptFunctionReference(State& state, int functionIndex);

	void init(State& state, std::string bytecodeFilename);
	void run(State& state);
	void drop(State& state);
}