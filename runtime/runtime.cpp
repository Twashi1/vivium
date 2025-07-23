#include "runtime.h"

namespace Runtime {
	void _submit(State& state)
	{
		_loadPipelines(state);
	}

	void _loadPipelines(State& state)
	{
		size_t pipelineCount = 0;
		serialiseRead(&pipelineCount, state.store);

		state.pipelines.resize(pipelineCount);

		for (size_t i = 0; i < pipelineCount; i++) {
			PipelineComponent component;

			readComponent(&component, state.store);

			// Turn component into a pipeline object now?
			state.pipelines[i] = _pipelineInstanceFromComponent(state, component);
		}
	}

	PipelineInstance _pipelineInstanceFromComponent(State& state, PipelineComponent& component)
	{
		// TODO
		PipelineInstance instance;
		instance.component = component;

		submitResource(state.manager, &instance.indexBuffer.reference, MemoryType::DEVICE, std::vector<BufferSpecification>(
			{ BufferSpecification(component.indexBuffer.size, component.indexBuffer.usage) }
		));

		submitResource(state.manager, &instance.vertexBuffer.reference, MemoryType::DEVICE, std::vector<BufferSpecification>(
			{ BufferSpecification(component.vertexBuffer.size, component.vertexBuffer.usage) }
		));

		ShaderSpecification vertexShader = compileShader(component.vertexShader.type, component.vertexShader.filename.c_str(), "out_vert.txt");
		ShaderSpecification fragmentShader = compileShader(component.vertexShader.type, component.fragmentShader.filename.c_str(), "out_fragtxt");

		submitResource(state.manager, &instance.vertexShader.reference, std::vector<ShaderSpecification>(
			{ vertexShader }
		));
		submitResource(state.manager, &instance.fragmentShader.reference, std::vector<ShaderSpecification>(
			{ fragmentShader }
		));

		std::vector<UniformData> uniformData;

		for (DescriptorSetItem const& item : component.descriptorSet.bindingData) {
			// TODO: switch on correct part
			DescriptorSetObjects object;

			// TODO: assumption...?
			//	need additional information to distinsguish uniform/storage/etc...
			object.type = UniformType::UNIFORM_BUFFER;

			submitResource(state.manager, &object.buffer.reference, MemoryType::UNIFORM, std::vector<BufferSpecification>({
				BufferSpecification(item.bufferPart.size, item.bufferPart.usage)
			}));

			uniformData.push_back(UniformData::fromBuffer(object.buffer.reference, item.bufferPart.size, 0));

			instance.descriptorObjects.push_back(object);
		}

		submitResource(state.manager, &instance.layout.reference, std::vector<DescriptorLayoutSpecification>({
				DescriptorLayoutSpecification(component.descriptorLayout.bindings)
		}));

		submitResource(state.manager, &instance.descriptor.reference, std::vector<DescriptorSetSpecification>({
			DescriptorSetSpecification(instance.layout.reference, uniformData)
		}));
		
		instance.indexCount = component.indexBuffer.data.size();

		return instance;
	}

	void _setup(State& state)
	{
		for (PipelineInstance& pipeline : state.pipelines) {
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

			// TODO: this is really bad and slow code... but
			//	reworking it is longer
			//	should create one large staging buffer and upload everything to that
			uint64_t vertexBufferSize = pipeline.component.vertexBuffer.data.size() * sizeof(float);
			uint64_t indexBufferSize = pipeline.component.indexBuffer.data.size() * sizeof(uint16_t);

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
				pipeline.component.vertexBuffer.data.data(),
				vertexBufferSize);
			cmdTransferBuffer(state.context, resource, vertexBufferSize, 0, pipeline.vertexBuffer.resource);

			std::memcpy(reinterpret_cast<uint8_t*>(stagingMapping) + vertexBufferSize,
				pipeline.component.indexBuffer.data.data(),
				indexBufferSize);
			cmdTransferBuffer(state.context, resource, indexBufferSize, vertexBufferSize, pipeline.indexBuffer.resource);

			contextEndTransfer(state.context, state.engine);

			_cmdFreeTransientStagingBuffer(state.engine, stagingBuffer, temporaryMemory);
		}
	}

	void _update(State& state)
	{
		// Upload any buffer data
		for (PipelineInstance& instance : state.pipelines) {
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
	}

	void _draw(State& state)
	{
		Perspective perspective = orthogonalPerspective2D(windowDimensions(state.window), F32x2(0.0f), 0.0f, 1.0f);

		for (PipelineInstance& instance : state.pipelines) {
			cmdBindPipeline(state.context, instance.pipeline.resource);
			cmdBindVertexBuffer(state.context, instance.vertexBuffer.resource);
			cmdBindIndexBuffer(state.context, instance.indexBuffer.resource);
			cmdBindDescriptorSet(state.context, instance.descriptor.resource, instance.pipeline.resource);
			cmdWritePushConstants(state.context, &perspective, sizeof(Perspective), 0, ShaderStage::VERTEX, instance.pipeline.resource);
			cmdDrawIndexed(state.context, instance.indexCount, 1);
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
		state.store.end();

		for (PipelineInstance& instance : state.pipelines) {
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