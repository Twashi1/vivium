#include "runtime.h"

namespace Runtime {
	void _submit(State& state)
	{
		_loadRegistry(state);

		for (ScriptMetadata& metadata : state.scripts) {
			_runScriptFunction(state, metadata.submitRef);
			_clearScriptFunctionReference(state, metadata.submitRef);
		}

		state.pipelineComponents = state.registry.createView<Owned<PipelineComponent>>();

		for (auto const& viewElement : state.pipelineComponents) {
			PipelineComponent const& pipeline = viewElement.get<PipelineComponent>();

			state.pipelineInstances.push_back(_pipelineInstanceFromComponent(state, pipeline));
		}
	}

	void _loadRegistry(State& state)
	{
		state.registry = Registry();

		// Begin loading in entities
		size_t numEntities = 0;
		serialiseRead(&numEntities, state.store);

		for (size_t i = 0; i < numEntities; i++) {
			Entity left = state.registry.create();

			Entity right;
			serialiseRead(&right, state.store);

			state.entityMap.insert({ right, left });

			VulkanComponent component;
			
			do {
				serialiseRead(&component, state.store);

				switch (component) {
				case VulkanComponent::BUFFER:
				{
					BufferComponent data;
					serialiseRead(&data, state.store);
					state.registry.addComponent(left, std::move(data));
					break;
				}
				case VulkanComponent::SHADER:
				{
					ShaderComponent data;
					serialiseRead(&data, state.store);
					state.registry.addComponent(left, std::move(data));
					break;
				}
				case VulkanComponent::BUFFER_LAYOUT:
				{
					BufferLayoutComponent data;
					serialiseRead(&data, state.store);
					state.registry.addComponent(left, std::move(data));
					break;
				}
				case VulkanComponent::DESCRIPTOR_LAYOUT:
				{
					DescriptorLayout data;
					serialiseRead(&data, state.store);
					state.registry.addComponent(left, std::move(data));
					break;
				}
				case VulkanComponent::DESCRIPTOR_SET:
				{
					DescriptorSet data;
					serialiseRead(&data, state.store);
					state.registry.addComponent(left, std::move(data));
					break;
				}
				case VulkanComponent::PIPELINE:
				{
					Pipeline data;
					serialiseRead(&data, state.store);
					state.registry.addComponent(left, std::move(data));
					break;
				}
				case VulkanComponent::ENTER_COMPONENT: break;
				default:
					VIVIUM_LOG(LogSeverity::ERROR, "Received unknown vulkan component when reading entity; badly formatted data?");
					break;
				}
			} while (component != VulkanComponent::ENTER_COMPONENT);
		}
	}

	PipelineInstance _pipelineInstanceFromComponent(State& state, PipelineComponent const& component)
	{
		// TODO
		PipelineInstance instance;
		instance.component = component;

		BufferComponent const& indexBuffer = state.registry.getComponent<BufferComponent>(instance.component.indexBuffer);
		BufferComponent const& vertexBuffer = state.registry.getComponent<BufferComponent>(instance.component.vertexBuffer);
		ShaderComponent const& fragmentShader = state.registry.getComponent<ShaderComponent>(instance.component.fragmentShader);
		ShaderComponent const& vertexShader = state.registry.getComponent<ShaderComponent>(instance.component.vertexShader);

		submitResource(state.manager, &instance.indexBuffer.reference, MemoryType::DEVICE, std::vector<BufferSpecification>(
			{ BufferSpecification(indexBuffer.data.size(), indexBuffer.usage)}
		));

		submitResource(state.manager, &instance.vertexBuffer.reference, MemoryType::DEVICE, std::vector<BufferSpecification>(
			{ BufferSpecification(vertexBuffer.data.size(), vertexBuffer.usage)}
		));

		ShaderSpecification vertexShaderSpec = compileShader(vertexShader.type, vertexShader.filename.c_str(), "out_vert.txt");
		ShaderSpecification fragmentShaderSpec = compileShader(fragmentShader.type, fragmentShader.filename.c_str(), "out_frag.txt");

		submitResource(state.manager, &instance.vertexShader.reference, std::vector<ShaderSpecification>(
			{ vertexShaderSpec }
		));
		submitResource(state.manager, &instance.fragmentShader.reference, std::vector<ShaderSpecification>(
			{ fragmentShaderSpec }
		));

		std::vector<UniformData> uniformData;
		
		DescriptorSetComponent const& descriptorSet = state.registry.getComponent<DescriptorSetComponent>(instance.component.descriptorSet);
		DescriptorLayoutComponent const& descriptorLayout = state.registry.getComponent<DescriptorLayoutComponent>(instance.component.descriptorLayout);

		for (Entity const& item : descriptorSet.bindingData) {
			// TODO: switch on correct part
			DescriptorSetObjects object;

			// TODO: assumption...?
			//	need additional information to distinsguish uniform/storage/etc...
			object.type = UniformType::UNIFORM_BUFFER;

			BufferComponent const& bufferPart = state.registry.getComponent<BufferComponent>(item);

			submitResource(state.manager, &object.buffer.reference, MemoryType::UNIFORM, std::vector<BufferSpecification>({
				BufferSpecification(bufferPart.data.size(), bufferPart.usage)
			}));

			uniformData.push_back(UniformData::fromBuffer(object.buffer.reference, bufferPart.data.size(), 0));

			instance.descriptorObjects.push_back(object);
		}

		submitResource(state.manager, &instance.layout.reference, std::vector<DescriptorLayoutSpecification>({
				DescriptorLayoutSpecification(descriptorLayout.bindings)
		}));

		submitResource(state.manager, &instance.descriptor.reference, std::vector<DescriptorSetSpecification>({
			DescriptorSetSpecification(instance.layout.reference, uniformData)
		}));
		
		instance.indexCount = indexBuffer.numElements;

		return instance;
	}

	void _loadScripts(State& state)
	{
		for (std::filesystem::directory_entry const& entry : std::filesystem::directory_iterator("vivium4/res/scripts/")) {
			if (!entry.is_regular_file()) { continue; }
			if (entry.path().extension() != ".lua") { continue; }

			state.scripts.push_back(_loadScript(state, entry.path().string()));
		}
	}

	ScriptMetadata _loadScript(State& state, std::string path)
	{
		ScriptMetadata metadata;
		metadata.name = path;
		metadata.submitRef = -1;
		metadata.setupRef = -1;
		metadata.updateRef = -1;
		metadata.drawRef = -1;
		metadata.dropRef = -1;

		if (luaL_dofile(state.L, path.c_str()) != LUA_OK) {
			VIVIUM_LOG(LogSeverity::ERROR, "Error loading script {}: {}", path, lua_tostring(state.L, -1));
			lua_pop(state.L, 1);

			return metadata;
		}

		if (!lua_istable(state.L, -1)) {
			VIVIUM_LOG(LogSeverity::ERROR, "Script ({}) should return table with functions", path);
			
			return metadata;
		}

		// TODO: fix this with a function
		// Look for function name, if its not a function pop it
		lua_getfield(state.L, -1, SCRIPT_FUNCTION_NAME_SUBMIT);
		if (!lua_isfunction(state.L, -1)) { lua_pop(state.L, 1); }
		else {
			metadata.submitRef = luaL_ref(state.L, LUA_REGISTRYINDEX);
		}

		lua_getfield(state.L, -1, SCRIPT_FUNCTION_NAME_SETUP);
		if (!lua_isfunction(state.L, -1)) { lua_pop(state.L, 1); }
		else {
			metadata.setupRef = luaL_ref(state.L, LUA_REGISTRYINDEX);
		}

		lua_getfield(state.L, -1, SCRIPT_FUNCTION_NAME_UPDATE);
		if (!lua_isfunction(state.L, -1)) { lua_pop(state.L, 1); }
		else {
			metadata.updateRef = luaL_ref(state.L, LUA_REGISTRYINDEX);
		}

		lua_getfield(state.L, -1, SCRIPT_FUNCTION_NAME_DRAW);
		if (!lua_isfunction(state.L, -1)) { lua_pop(state.L, 1); }
		else {
			metadata.drawRef = luaL_ref(state.L, LUA_REGISTRYINDEX);
		}

		lua_getfield(state.L, -1, SCRIPT_FUNCTION_NAME_DROP);
		if (!lua_isfunction(state.L, -1)) { lua_pop(state.L, 1); }
		else {
			metadata.dropRef = luaL_ref(state.L, LUA_REGISTRYINDEX);
		}

		// Pop the table
		lua_pop(state.L, 1);

		return metadata;
	}

	void _runScriptFunction(State& state, int functionIndex)
	{
		if (functionIndex < 0) return;

		lua_rawgeti(state.L, LUA_REGISTRYINDEX, functionIndex);
		if (!lua_isfunction(state.L, -1)) {
			VIVIUM_LOG(LogSeverity::ERROR, "Failed to get function by index {}", functionIndex);
			lua_pop(state.L, 1);

			return;
		}

		// TODO: check returns nothing (nil?)

		if (lua_pcall(state.L, 0, 0, 0) != LUA_OK) {
			// TODO: script name for debug
			VIVIUM_LOG(LogSeverity::ERROR, "Failed to run function: {}", lua_tostring(state.L, -1));
			lua_pop(state.L, 1);
			return;
		}
	}

	void _clearScriptFunctionReference(State& state, int functionIndex)
	{
		luaL_unref(state.L, LUA_REGISTRYINDEX, functionIndex);
	}

	void _setup(State& state)
	{
		for (PipelineInstance& pipeline : state.pipelineInstances) {
			convertResourceReference(state.manager, pipeline.vertexBuffer);
			convertResourceReference(state.manager, pipeline.indexBuffer);

			convertResourceReference(state.manager, pipeline.vertexShader);
			convertResourceReference(state.manager, pipeline.fragmentShader);

			for (DescriptorSetObjects& object : pipeline.descriptorObjects) {
				// TODO: switch on type
				convertResourceReference(state.manager, object.buffer);
			}

			convertResourceReference(state.manager, pipeline.layout);
			convertResourceReference(state.manager, pipeline.descriptor);
			convertResourceReference(state.manager, pipeline.pipeline);

			BufferComponent const& indexBuffer = state.registry.getComponent<BufferComponent>(pipeline.component.indexBuffer);
			BufferComponent const& vertexBuffer = state.registry.getComponent<BufferComponent>(pipeline.component.vertexBuffer);

			// TODO: this is really bad and slow code... but
			//	reworking it is longer
			//	should create one large staging buffer and upload everything to that
			uint64_t vertexBufferSize = vertexBuffer.data.size();
			uint64_t indexBufferSize = indexBuffer.data.size();

			// Create staging for vertices and indices
			VkDeviceMemory temporaryMemory;
			VkBuffer stagingBuffer;
			void* stagingMapping;
			// Staging buffer both for vertex data and index data
			_cmdCreateTransientStagingBuffer(
				state.engine,
				&stagingBuffer,
				&temporaryMemory,
				vertexBufferSize + indexBufferSize,
				&stagingMapping);

			Buffer resource;
			resource.buffer = stagingBuffer;
			resource.mapping = stagingMapping;

			contextBeginTransfer(state.context);

			std::memcpy(stagingMapping,
				vertexBuffer.data.data(),
				vertexBufferSize);
			cmdTransferBuffer(state.context, resource, vertexBufferSize, 0, pipeline.vertexBuffer.resource);

			std::memcpy(reinterpret_cast<uint8_t*>(stagingMapping) + vertexBufferSize,
				indexBuffer.data.data(),
				indexBufferSize);
			cmdTransferBuffer(state.context, resource, indexBufferSize, vertexBufferSize, pipeline.indexBuffer.resource);

			contextEndTransfer(state.context, state.engine);

			_cmdFreeTransientStagingBuffer(state.engine, stagingBuffer, temporaryMemory);
		}

		for (ScriptMetadata& metadata : state.scripts) {
			_runScriptFunction(state, metadata.setupRef);
			_clearScriptFunctionReference(state, metadata.setupRef);
		}
	}

	void _update(State& state)
	{
		// Upload any buffer data
		for (PipelineInstance& instance : state.pipelineInstances) {
			for (DescriptorSetObjects& object : instance.descriptorObjects) {
				switch (object.type) {
				case UniformType::UNIFORM_BUFFER:
				case UniformType::STORAGE_BUFFER:
				case UniformType::DYNAMIC_UNIFORM_BUFFER:
					setBuffer(object.buffer.resource, 0, object.bufferComponent.data.data(), object.bufferComponent.data.size() * sizeof(float));
					break;
				case UniformType::TEXTURE:
				case UniformType::FRAMEBUFFER:
					break;
				default:
					VIVIUM_ASSERT(false, "Invalid uniform type");
					break;
				}
			}
		}

		for (ScriptMetadata& metadata : state.scripts) {
			_runScriptFunction(state, metadata.updateRef);
		}
	}

	void _draw(State& state)
	{
		Perspective perspective = orthogonalPerspective2D(windowDimensions(state.window), F32x2(0.0f), 0.0f, 1.0f);

		// TODO: don't draw any pipeline by default? make draw schedule it?
		for (PipelineInstance& instance : state.pipelineInstances) {
			cmdBindPipeline(state.context, instance.pipeline.resource);
			cmdBindVertexBuffer(state.context, instance.vertexBuffer.resource);
			cmdBindIndexBuffer(state.context, instance.indexBuffer.resource);
			cmdBindDescriptorSet(state.context, instance.descriptor.resource, instance.pipeline.resource);
			cmdWritePushConstants(state.context, &perspective, sizeof(Perspective), 0, ShaderStage::VERTEX, instance.pipeline.resource);
			cmdDrawIndexed(state.context, instance.indexCount, 1);
		}

		for (ScriptMetadata& metadata : state.scripts) {
			_runScriptFunction(state, metadata.drawRef);
		}
	}

	void init(State& state, std::string bytecodeFilename)
	{
		_logInit();
		_fontInit();

		state.engine = createEngine(EngineOptions{});
		state.window = createWindow(WindowOptions{}, state.engine);
		state.manager = createManager();
		state.context = createCommandContext(state.engine);
		state.store.begin(bytecodeFilename, true);
		state.L = luaL_newstate();
		luaL_openlibs(state.L);

		Input::init(state.window);

		_submit(state);
		allocateManager(state.manager, state.engine);
	
		_setup(state);
		clearManagerReferences(state.manager);
	}

	void run(State& state)
	{
		Input::update(state.window);
		_update(state);

		engineBeginFrame(state.engine, state.context);

		_draw(state);
		
		engineEndFrame(state.engine);
	}

	void drop(State& state)
	{
		for (ScriptMetadata& metadata : state.scripts) {
			_runScriptFunction(state, metadata.dropRef);
			_clearScriptFunctionReference(state, metadata.dropRef);
			_clearScriptFunctionReference(state, metadata.updateRef);
			_clearScriptFunctionReference(state, metadata.drawRef);
		}

		state.store.end();
		lua_close(state.L);

		for (PipelineInstance& instance : state.pipelineInstances) {
			dropBuffer(instance.vertexBuffer.resource, state.engine);
			dropBuffer(instance.indexBuffer.resource, state.engine);

			for (DescriptorSetObjects& item : instance.descriptorObjects) {
				switch (item.type) {
				case UniformType::UNIFORM_BUFFER:
				case UniformType::STORAGE_BUFFER:
				case UniformType::DYNAMIC_UNIFORM_BUFFER:
					dropBuffer(item.buffer.resource, state.engine);
					break;
				default:
					VIVIUM_LOG(LogSeverity::FATAL, "Unsupported item type");
					break;
				}
			}

			dropPipeline(instance.pipeline.resource, state.engine);
		}

		dropManager(state.manager, state.engine);
		dropCommandContext(state.context, state.engine);

		dropWindow(state.window, state.engine);
		dropEngine(state.engine);

		_fontTerminate();
	}
}