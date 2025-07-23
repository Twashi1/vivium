#pragma once

#include "../vivium4/vivium4.h"
#include "../engine/state.h"

using namespace Vivium;

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

	struct State {
		Engine engine;
		Window window;
		CommandContext context;
		ResourceManager manager;

		std::vector<PipelineInstance> pipelines;

		SerialiserFileInterface store;
	};

	void _submit(State& state);
	void _setup(State& state);
	void _update(State& state);
	void _draw(State& state);

	void _loadPipelines(State& state);
	PipelineInstance _pipelineInstanceFromComponent(State& state, PipelineComponent& component);

	void init(State& state, std::string bytecodeFilename);
	void run(State& state);
	void drop(State& state);
}