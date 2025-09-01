#include "runtime.h"

namespace Runtime {
	void _submit(State& state)
	{
		_loadRegistry(state);
		state.luaContext.registry = &state.registry;
		state.luaContext.stage = Stage::SUBMIT;
		state.luaContext.entityMap = &state.entityMap;
		state.luaContext.pipelineInstances = &state.pipelineInstances;

		for (ScriptMetadata& metadata : state.scripts) {
			_runScriptFunction(state, metadata.submitRef);
			_clearScriptFunctionReference(state, metadata.submitRef);
		}

		state.pipelineComponents = state.registry.createView<Owned<PipelineComponent>>();

		for (auto const& viewElement : state.pipelineComponents) {
			PipelineComponent const& pipeline = viewElement.get<PipelineComponent>();

			state.pipelineInstances.push_back(_pipelineInstanceFromComponent(state, pipeline, viewElement.entity));
		}
	}

	void _loadRegistry(State& state)
	{
		state.registry = Registry();

		// Begin loading in entities
		std::vector<Entity> entities;
		serialiseRead(&entities, state.store);

		for (size_t i = 0; i < entities.size(); i++) {
			Entity left = state.registry.create();

			Entity right = entities[i];

			state.entityMap.insert({ right, left });
		}

		serialiseRead(&state.registry, state.store);
	}

	PipelineInstance _pipelineInstanceFromComponent(State& state, PipelineComponent const& component, Entity entity)
	{
		// TODO
		PipelineInstance instance;
		instance.component = component;
		instance.instanceCount = 1;
		instance.entity = entity;

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

		for (Entity const item : descriptorSet.bindingData) {
			// TODO: switch on correct part
			DescriptorSetObjects object;

			object.entity = item;

			// Figure out if we have a buffer/texture/etc.
			if (state.registry.hasComponent<BufferComponent>(item)) {
				BufferComponent const& bufferPart = state.registry.getComponent<BufferComponent>(item);

				// TODO: cannot distinguish dynamic uniform buffer
				if (bufferPart.usage == BufferUsage::STORAGE) {
					object.type = UniformType::STORAGE_BUFFER;
				}
				else {
					object.type = UniformType::UNIFORM_BUFFER;
				}

				submitResource(state.manager, &object.buffer.reference, MemoryType::UNIFORM, std::vector<BufferSpecification>({
					BufferSpecification(bufferPart.data.size(), bufferPart.usage)
				}));

				uniformData.push_back(UniformData::fromBuffer(object.buffer.reference, bufferPart.data.size(), 0));
			}
			else if (state.registry.hasComponent<TextureComponent>(item)) {
				TextureComponent const& texturePart = state.registry.getComponent<TextureComponent>(item);

				// TODO: Some management/resource system to de-duplicate textures
				submitResource(state.manager, &object.texture.reference, std::vector<TextureSpecification>({
					TextureSpecification::fromImageFile(texturePart.filename.c_str(), texturePart.format, texturePart.filter)
				}));

				object.type = UniformType::TEXTURE;

				uniformData.push_back(UniformData::fromTexture(object.texture.reference));
			}
			else {
				VIVIUM_LOG(LogSeverity::ERROR, "Failed to detect valid descriptor item on entity {}", static_cast<uint32_t>(item));
			}

			instance.descriptorObjects.push_back(object);
		}

		submitResource(state.manager, &instance.layout.reference, std::vector<DescriptorLayoutSpecification>({
				DescriptorLayoutSpecification(descriptorLayout.bindings)
		}));

		submitResource(state.manager, &instance.descriptor.reference, std::vector<DescriptorSetSpecification>({
			DescriptorSetSpecification(instance.layout.reference, uniformData)
		}));

		BufferLayout bufferLayout = BufferLayout::fromTypes(state.registry.getComponent<BufferLayoutComponent>(component.bufferLayout).types);

		submitResource(state.manager, &instance.pipeline.reference, std::vector<PipelineSpecification>({
			PipelineSpecification::fromWindow(
				std::vector<ShaderReference>({ instance.vertexShader.reference, instance.fragmentShader.reference }),
				bufferLayout,
				std::vector<DescriptorLayoutReference>({ instance.layout.reference }),
				std::vector<PushConstant>({ PushConstant(ShaderStage::VERTEX, 0, sizeof(Perspective)) }),
				state.window
			)
		}));
		
		// TODO: this is the single place we need the number of elements...
		instance.indexCount = 6;

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
		state.luaContext.stage = Stage::SETUP;

		for (PipelineInstance& pipeline : state.pipelineInstances) {
			convertResourceReference(state.manager, pipeline.vertexBuffer);
			convertResourceReference(state.manager, pipeline.indexBuffer);

			convertResourceReference(state.manager, pipeline.vertexShader);
			convertResourceReference(state.manager, pipeline.fragmentShader);

			for (DescriptorSetObjects& object : pipeline.descriptorObjects) {
				// TODO: switch on type
				switch (object.type) {
				case UniformType::UNIFORM_BUFFER:
				case UniformType::DYNAMIC_UNIFORM_BUFFER:
				case UniformType::STORAGE_BUFFER:
					convertResourceReference(state.manager, object.buffer); break;
				case UniformType::TEXTURE:
					convertResourceReference(state.manager, object.texture); break;
				case UniformType::FRAMEBUFFER:
					convertResourceReference(state.manager, object.framebuffer); break;
				default:
					VIVIUM_LOG(LogSeverity::ERROR, "Invalid descriptor uniform type"); break;
				}
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

			// Drop shaders since no longer needed
			// TODO: could drop descriptor layout maybe? look deeper
			dropShader(pipeline.vertexShader.resource, state.engine);
			dropShader(pipeline.fragmentShader.resource, state.engine);

			// Upload the vulkan version of objects
			state.registry.addComponent<Buffer>(pipeline.component.indexBuffer, pipeline.indexBuffer.resource);
			state.registry.addComponent<Buffer>(pipeline.component.vertexBuffer, pipeline.vertexBuffer.resource);
			state.registry.addComponent<DescriptorSet>(pipeline.component.descriptorSet, pipeline.descriptor.resource);
			state.registry.addComponent<Pipeline>(pipeline.entity, pipeline.pipeline.resource);
			// Note we don't add shaders because we drop them
			//	and we don't add descriptor layouts because there isn't a good use case for them?

			// TODO: descriptor set objects
			for (DescriptorSetObjects& object : pipeline.descriptorObjects) {
				switch (object.type) {
				case UniformType::UNIFORM_BUFFER:
				case UniformType::STORAGE_BUFFER:
				case UniformType::DYNAMIC_UNIFORM_BUFFER:
					state.registry.addComponent<Buffer>(object.entity, object.buffer.resource);
					break;
				case UniformType::TEXTURE:
					state.registry.addComponent<Texture>(object.entity, object.texture.resource);
					break;
				// TODO: framebuffer might be more difficult to control?
				case UniformType::FRAMEBUFFER:
					state.registry.addComponent<Framebuffer>(object.entity, object.framebuffer.resource);
					break;
				}
			}
		}

		for (ScriptMetadata& metadata : state.scripts) {
			_runScriptFunction(state, metadata.setupRef);
			_clearScriptFunctionReference(state, metadata.setupRef);
		}
	}

	void _update(State& state)
	{
		state.luaContext.stage = Stage::UPDATE;

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
		state.luaContext.stage = Stage::DRAW;

		Perspective perspective = orthogonalPerspective2D(windowDimensions(state.window), F32x2(0.0f), 0.0f, 1.0f);

		// TODO: don't draw any pipeline by default? make draw schedule it?
		for (PipelineInstance& instance : state.pipelineInstances) {
			cmdBindPipeline(state.context, instance.pipeline.resource);
			cmdBindVertexBuffer(state.context, instance.vertexBuffer.resource);
			cmdBindIndexBuffer(state.context, instance.indexBuffer.resource);
			cmdBindDescriptorSet(state.context, instance.descriptor.resource, instance.pipeline.resource);
			cmdWritePushConstants(state.context, &perspective, sizeof(Perspective), 0, ShaderStage::VERTEX, instance.pipeline.resource);
			cmdDrawIndexed(state.context, instance.indexCount, instance.instanceCount);
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

		_loadScripts(state);
		_loadLuaObjects(state);

		_submit(state);
		allocateManager(state.manager, state.engine);
	
		_setup(state);
		clearManagerReferences(state.manager);
	}

	void run(State& state)
	{
		while (windowIsOpen(state.window, state.engine)) {
			engineBeginFrame(state.engine, state.context);

			Input::update(state.window);

			_update(state);

			windowBeginFrame(state.window, state.context, state.engine);
			windowBeginRender(state.window);

			_draw(state);

			windowEndRender(state.window);
			windowEndFrame(state.window, state.engine);

			engineEndFrame(state.engine);
		}

    VIVIUM_LOG(LogSeverity::DEBUG, "Window is closing now");
	} 

	void drop(State& state)
	{
		state.luaContext.stage = Stage::DROP;

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
				case UniformType::TEXTURE:
					dropTexture(item.texture.resource, state.engine);
					break;
				default:
					VIVIUM_LOG(LogSeverity::FATAL, "Unsupported item type");
					break;
				}
			}

			dropDescriptorLayout(instance.layout.resource, state.engine);
			dropPipeline(instance.pipeline.resource, state.engine);
		}

		dropManager(state.manager, state.engine);
		dropCommandContext(state.context, state.engine);

		dropWindow(state.window, state.engine);
		dropEngine(state.engine);

		_fontTerminate();
	}

	void _pushLuaFunction(State& state, lua_CFunction function, std::string_view name)
	{
		lua_pushlightuserdata(state.L, &state.luaContext);
		lua_pushcclosure(state.L, function, 1);
		lua_setglobal(state.L, name.data());
	}
	
	void _loadLuaObjects(State& state)
	{
		_loadLuaComponentsEnum(state);
		_loadLuaDataTypesEnum(state);
		_loadLuaEntity(state);

		_pushLuaFunction(state, _luaCreateEntity, "vCreateEntity");
		
		_pushLuaFunction(state, _luaGetComponent, "vGetComponent");
		_pushLuaFunction(state, _luaSetBufferData, "vSetBufferData");
		_pushLuaFunction(state, _luaEntityID, "vGetEntityByID");
		_pushLuaFunction(state, _luaDrawIndex, "vDrawIndex");

		_pushLuaFunction(state, _luaCursorPosition, "vCursorPosition");
		_pushLuaFunction(state, _luaIsLeftClick, "vIsLeftClick");
		_pushLuaFunction(state, _luaIsRightClick, "vIsRightClick");
	}

	void _loadLuaEntity(State& state)
	{
		// Guard against multiple initialisations
		if (luaL_newmetatable(state.L, ENTITY_TABLE_NAME)) {
			lua_pushcfunction(state.L, _luaBlock);
			lua_setfield(state.L, -2, "__index");

			lua_pushcfunction(state.L, _luaBlock);
			lua_setfield(state.L, -2, "__newindex");

			lua_pushcfunction(state.L, _luaEntityString);
			lua_setfield(state.L, -2, "__tostring");

			// Garbage collection function
			lua_pushcfunction(state.L, _luaEntityDrop);
			lua_setfield(state.L, -2, "__gc");

			// Protect metatable
			lua_pushstring(state.L, "Metatable access denied");
			lua_setfield(state.L, -2, "__metatable");
		}

		lua_pop(state.L, 1);
	}

	void _loadLuaComponentsEnum(State& state)
	{
		lua_pushinteger(state.L, static_cast<uint32_t>(VulkanComponent::BUFFER));
		lua_setglobal(state.L, "vBUFFER");

		lua_pushinteger(state.L, static_cast<uint32_t>(VulkanComponent::BUFFER_LAYOUT));
		lua_setglobal(state.L, "vBUFFER_LAYOUT");

		lua_pushinteger(state.L, static_cast<uint32_t>(VulkanComponent::DESCRIPTOR_LAYOUT));
		lua_setglobal(state.L, "vDESCRIPTOR_LAYOUT");

		lua_pushinteger(state.L, static_cast<uint32_t>(VulkanComponent::SHADER));
		lua_setglobal(state.L, "vSHADER");

		lua_pushinteger(state.L, static_cast<uint32_t>(VulkanComponent::DESCRIPTOR_SET));
		lua_setglobal(state.L, "vDESCRIPTOR_SET");

		lua_pushinteger(state.L, static_cast<uint32_t>(VulkanComponent::TEXTURE));
		lua_setglobal(state.L, "vTEXTURE");

		lua_pushinteger(state.L, static_cast<uint32_t>(VulkanComponent::PIPELINE));
		lua_setglobal(state.L, "vPIPELINE");
	}

	void _loadLuaDataTypesEnum(State& state)
	{
		lua_pushinteger(state.L, static_cast<uint32_t>(LuaDataType::UINT8));
		lua_setglobal(state.L, "vUINT8");

		lua_pushinteger(state.L, static_cast<uint32_t>(LuaDataType::UINT16));
		lua_setglobal(state.L, "vUINT16");

		lua_pushinteger(state.L, static_cast<uint32_t>(LuaDataType::UINT32));
		lua_setglobal(state.L, "vUINT32");

		lua_pushinteger(state.L, static_cast<uint32_t>(LuaDataType::FLOAT));
		lua_setglobal(state.L, "vFLOAT");

		lua_pushinteger(state.L, static_cast<uint32_t>(LuaDataType::VEC2));
		lua_setglobal(state.L, "vVEC2");

		lua_pushinteger(state.L, static_cast<uint32_t>(LuaDataType::VEC3));
		lua_setglobal(state.L, "vVEC3");
	}

	int _luaBlock(lua_State* L)
	{
		return luaL_error(L, "Vivium access denied");
	}

	int _luaCreateEntity(lua_State* L)
	{
		LuaContext* context = static_cast<LuaContext*>(lua_touserdata(L, lua_upvalueindex(1)));
		VIVIUM_ASSERT(context != nullptr, "Missing lua context");

		// TODO: we can't access the registry from here
		Entity newLuaEntity = context->registry->create();

		void* userdata = lua_newuserdata(L, sizeof(Entity));
		Entity* entity = new (userdata) Entity;
		*entity = newLuaEntity;

		luaL_getmetatable(L, ENTITY_TABLE_NAME);
		lua_setmetatable(L, -2);

		return 1;
	}
	
	int _luaEntityDrop(lua_State* L)
	{
		// No-op for now
		return 0;
	}
	
	int _luaEntityString(lua_State* L)
	{
		Entity entity = *static_cast<Entity*>(luaL_checkudata(L, 1, ENTITY_TABLE_NAME));
		std::string representation = std::format("<Entity: id={}>", static_cast<uint32_t>(entity));
		lua_pushlstring(L, representation.c_str(), representation.length());

		return 1;
	}

	int _luaEntityID(lua_State* L)
	{
		LuaContext* context = static_cast<LuaContext*>(lua_touserdata(L, lua_upvalueindex(1)));
		VIVIUM_ASSERT(context != nullptr, "Missing lua context");

		Entity lookup = static_cast<Entity>(luaL_checkinteger(L, 1));
		Entity result = context->entityMap->at(lookup);

		void* userdata = lua_newuserdata(L, sizeof(Entity));
		Entity* entity = new (userdata) Entity;
		*entity = result;

		luaL_getmetatable(L, ENTITY_TABLE_NAME);
		lua_setmetatable(L, -2);

		return 1;
	}

	int _luaGetComponent(lua_State* L)
	{
		LuaContext* context = static_cast<LuaContext*>(lua_touserdata(L, lua_upvalueindex(1)));
		VIVIUM_ASSERT(context != nullptr, "Missing lua context");

		VulkanComponent componentType = static_cast<VulkanComponent>(luaL_checkinteger(L, 1));

		Entity* entity = static_cast<Entity*>(luaL_checkudata(L, 2, ENTITY_TABLE_NAME));
		VIVIUM_ASSERT(entity != nullptr, "Expected entity as second argument");

		// TODO: get component
		//	and then for each type, we have a function to wrap it into a lua context?
		switch (componentType) {
		case VulkanComponent::BUFFER:
			break;
		case VulkanComponent::SHADER:
			break;
		case VulkanComponent::BUFFER_LAYOUT:
			break;
		case VulkanComponent::DESCRIPTOR_LAYOUT:
			break;
		case VulkanComponent::DESCRIPTOR_SET:
			break;
		case VulkanComponent::PIPELINE:
			break;
		default: VIVIUM_LOG(LogSeverity::FATAL, "Received invalid component type"); break;
		}

		// TODO: temporary just send them a random number
		lua_pushinteger(L, 5);

		return 1;
	}
	
	int _luaSetBufferData(lua_State* L)
	{
		LuaContext* context = static_cast<LuaContext*>(lua_touserdata(L, lua_upvalueindex(1)));
		VIVIUM_ASSERT(context != nullptr, "Missing lua context");

		luaL_checktype(L, 1, LUA_TTABLE);

		LuaDataType dataType = static_cast<LuaDataType>(luaL_checkinteger(L, 2));

		Entity* entity = static_cast<Entity*>(luaL_checkudata(L, 3, ENTITY_TABLE_NAME));
		VIVIUM_ASSERT(entity != nullptr, "Expected entity as third argument");

		std::vector<uint8_t> newBufferData;

		// Get length of array of lua table
		lua_Integer len = luaL_len(L, 1);

		for (lua_Integer i = 1; i <= len; i++) {
			// Push value to stack
			lua_geti(L, 1, i);

			if (lua_isnumber(L, -1)) {
				// TODO: relies on assumption that this is large enough
				//	to store any type
				uint8_t elementData[16];
				uint64_t typeSize;

				// TODO: really error prone and ugly
				switch (dataType) {
				case LuaDataType::UINT8:
					*reinterpret_cast<uint8_t*>(elementData) = static_cast<uint8_t>(lua_tointeger(L, -1));
					typeSize = sizeof(uint8_t);
					break;
				case LuaDataType::UINT16:
					*reinterpret_cast<uint16_t*>(elementData) = static_cast<uint16_t>(lua_tointeger(L, -1));
					typeSize = sizeof(uint16_t);
					break;
				case LuaDataType::UINT32:
					*reinterpret_cast<uint32_t*>(elementData) = static_cast<uint32_t>(lua_tointeger(L, -1));
					typeSize = sizeof(uint32_t);
					break;
				case LuaDataType::FLOAT:
					*reinterpret_cast<float*>(elementData) = static_cast<float>(lua_tonumber(L, -1));
					typeSize = sizeof(float);
					break;
				case LuaDataType::VEC2: break;
				case LuaDataType::VEC3: break;
				default: break;
				}

				// TODO: bad code... same reason as above
				newBufferData.resize(newBufferData.size() + typeSize);
				memcpy(newBufferData.data() + newBufferData.size() - typeSize, elementData, typeSize);
			}

			lua_pop(L, 1);
		}

		// TODO: lots of code
		// 1. if we're in the submit phase, just change the buffer data and number of elements (count?)
		// 2. if we're in a phase after, we ensure we have the same number of elements and length of data
		//	then we change the buffer data
		if (context->stage == Stage::SUBMIT) {
			// Get the buffer component and modify buffer data/number of elements
			BufferComponent& buffer = context->registry->getComponent<BufferComponent>(*entity);

			buffer.data = std::move(newBufferData);
		}
		else if (context->stage == Stage::SETUP || context->stage == Stage::UPDATE || context->stage == Stage::DRAW) {
			// TODO: behaviour for other valid states, behaviour for invalid states
			// Update the Buffer itself with the data
			Buffer& buffer = context->registry->getComponent<Buffer>(*entity);
			setBuffer(buffer, 0, newBufferData.data(), newBufferData.size());
		}

		return 0;
	}
	
	int _luaDrawIndex(lua_State* L)
	{
		LuaContext* context = static_cast<LuaContext*>(lua_touserdata(L, lua_upvalueindex(1)));
		VIVIUM_ASSERT(context != nullptr, "Missing lua context");

		Entity* entity = static_cast<Entity*>(luaL_checkudata(L, 1, ENTITY_TABLE_NAME));
		VIVIUM_ASSERT(entity != nullptr, "Expected entity as first argument");

		uint64_t instanceCount = static_cast<uint64_t>(luaL_checkinteger(L, 2));

		VIVIUM_ASSERT(context->stage == Stage::DRAW, "Not in draw stage");
		VIVIUM_ASSERT(context->registry->hasComponent<Pipeline>(*entity), "Entity isn't a rendering pipeline");

		// TODO: somehow grab the pipeline instance
		//	draw the pipeline instance
		bool foundPipeline = false;

		for (PipelineInstance& instance : *(context->pipelineInstances)) {
			if (instance.entity == *entity) {
				instance.instanceCount = instanceCount;
				foundPipeline = true;
			}
		}

		VIVIUM_ASSERT(foundPipeline, "Couldn't find pipeline instance for entity {}", *entity);

		return 0;
	}

	int _luaCursorPosition(lua_State* L)
	{
		F32x2 cursorPos = Input::getCursor();

		lua_newtable(L);

		lua_pushnumber(L, cursorPos.x);
		lua_rawseti(L, -2, 1);

		lua_pushnumber(L, cursorPos.y);
		lua_rawseti(L, -2, 2);

		return 1;
	}

	int _luaIsLeftClick(lua_State* L)
	{
		lua_pushboolean(L, Input::get(Input::BTN_1).state == Input::RELEASE);

		return 1;
	}

	int _luaIsRightClick(lua_State* L)
	{
		lua_pushboolean(L, Input::get(Input::BTN_2).state == Input::RELEASE);

		return 1;
	}
}
