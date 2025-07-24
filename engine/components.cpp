#include "components.h"

std::string getString(VulkanComponent component) {
	switch (component) {
	case VulkanComponent::PIPELINE: return "Pipeline";
	case VulkanComponent::BUFFER_LAYOUT: return "Buffer Layout";
	case VulkanComponent::DESCRIPTOR_LAYOUT: return "Descriptor Layout";
	case VulkanComponent::SHADER: return "Shader";
	case VulkanComponent::BUFFER: return "Buffer";
	case VulkanComponent::DESCRIPTOR_SET: return "Descriptor Set";
	case VulkanComponent::ENTER_COMPONENT: return "Enter component";
	default: return "Unknown";
	}
}

UniformBinding getValue(UniformBindingEntry const& entry)
{
	UniformBinding binding;

	binding.slot = getValue(entry.slotEntry);
	binding.stage = getValue(entry.stageEntry);
	binding.type = getValue(entry.typeEntry);
	
	return binding;
}

UniformBindingEntry submitEntry(EntrySpecification<UniformBindingEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager)
{
	UniformBindingEntry entry;

	entry.slotEntry = submitEntry(EntrySpecification<IntegerTextEntry>("Enter slot"), guiContext, resourceManager);

	entry.stageEntry = submitEntry(EntrySpecification<ObjectEntry<ShaderStage>>(
		ShaderStage::VERTEX,
		std::vector<ShaderStage>({ ShaderStage::VERTEX, ShaderStage::FRAGMENT })
	), guiContext, resourceManager);

	entry.typeEntry = submitEntry(EntrySpecification<ObjectEntry<UniformType>>(
		UniformType::STORAGE_BUFFER,
		std::vector<UniformType>({ UniformType::TEXTURE, UniformType::STORAGE_BUFFER, UniformType::UNIFORM_BUFFER })
	), guiContext, resourceManager);

	entry.entryContainer = createContainer(guiContext, ContainerSpecification(
		nullGUIParent(),
		ContainerOrdering::VERTICAL,
		OffsetMethod::EXTENT
	));
	entry.base = entry.entryContainer.base;

	return entry;
}

void setupEntry(UniformBindingEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext)
{
	// TODO: setup properties of everything

	setupEntry(entry.slotEntry, manager, engine, context, guiContext);
	setupEntry(entry.stageEntry, manager, engine, context, guiContext);
	setupEntry(entry.typeEntry, manager, engine, context, guiContext);

	addChild(entry.entryContainer.base, { &entry.slotEntry.base, 1 }, guiContext);
	addChild(entry.entryContainer.base, { &entry.stageEntry.base, 1 }, guiContext);
	addChild(entry.entryContainer.base, { &entry.typeEntry.base, 1 }, guiContext);

	properties(entry.entryContainer.base, guiContext).dimensions = F32x2(1.0f);
	properties(entry.entryContainer.base, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.entryContainer.base, guiContext).centerY = GUIAnchor::TOP;

	properties(entry.slotEntry.base, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.stageEntry.base, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.typeEntry.base, guiContext).anchorY = GUIAnchor::TOP;

	properties(entry.slotEntry.base, guiContext).centerY = GUIAnchor::TOP;
	properties(entry.stageEntry.base, guiContext).centerY = GUIAnchor::TOP;
	properties(entry.typeEntry.base, guiContext).centerY = GUIAnchor::TOP;
}

void updateEntry(UniformBindingEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
{
	updateEntry(entry.slotEntry, guiContext, engine, context);
	updateEntry(entry.stageEntry, guiContext, engine, context);
	updateEntry(entry.typeEntry, guiContext, engine, context);
}

void submitEntries(std::span<UniformBindingEntry*> const entries, GUIContext& guiContext)
{
	for (UniformBindingEntry* entry : entries) {
		IntegerTextEntry* slots[] = { &entry->slotEntry };
		ObjectEntry<UniformType>* types[] = { &entry->typeEntry };
		ObjectEntry<ShaderStage>* stages[] = { &entry->stageEntry };

		submitEntries(slots, guiContext);
		submitEntries<UniformType>(types, guiContext);
		submitEntries<ShaderStage>(stages, guiContext);
	}
}

void dropEntry(UniformBindingEntry& entry, Engine& engine, GUIContext& guiContext)
{
	dropEntry(entry.slotEntry, engine, guiContext);
	dropEntry(entry.typeEntry, engine, guiContext);
	dropEntry(entry.stageEntry, engine, guiContext);
}

BufferLayoutComponent getValue(BufferLayoutEntry const& entry)
{
	return BufferLayoutComponent(getValue(entry.typesEntry));
}

BufferLayoutEntry submitEntry(EntrySpecification<BufferLayoutEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager)
{
	EntrySpecification<ListEntry<ObjectEntry<ShaderDataType>>> entrySpec;
	entrySpec.maxEntries = 4;
	EntrySpecification<ObjectEntry<ShaderDataType>> objectEntrySpec;
	objectEntrySpec.defaultValue = ShaderDataType::VEC2;
	objectEntrySpec.options = {
		ShaderDataType::VEC2,
		ShaderDataType::VEC3,
		ShaderDataType::FLOAT,
		ShaderDataType::INT
	};
	entrySpec.valueSpecification = &objectEntrySpec;

	BufferLayoutEntry bufferLayout;

	bufferLayout.base = createGUIElement(guiContext, GUIElementType::ENTRY);
	bufferLayout.typesEntry = submitEntry(entrySpec, guiContext, resourceManager);

	addChild(bufferLayout.base, { &bufferLayout.typesEntry.base, 1 }, guiContext);

	return bufferLayout;
}

void setupEntry(BufferLayoutEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext)
{
	setupEntry(entry.typesEntry, manager, engine, context, guiContext);
}

void updateEntry(BufferLayoutEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
{
	updateEntry(entry.typesEntry, guiContext, engine, context);
}

void submitEntries(std::span<BufferLayoutEntry*> const entries, GUIContext& guiContext)
{
	for (BufferLayoutEntry* entry : entries) {
		ListEntry<ObjectEntry<ShaderDataType>>* listEntries[] = { &entry->typesEntry };

		submitEntries<ObjectEntry<ShaderDataType>>(listEntries, guiContext);
	}
}

void dropEntry(BufferLayoutEntry& entry, Engine& engine, GUIContext& guiContext)
{
	dropEntry(entry.typesEntry, engine, guiContext);
}

ShaderComponent getValue(ShaderEntry const& entry)
{
	//std::string shaderFilename = getValue(entry.filenameEntry);
	//std::string::iterator dot = std::find(shaderFilename.begin(), shaderFilename.end(), '.');

	//// get text after dot, re-append to text before dot with _, and finally .spv
	//std::string extension = std::string(std::next(dot), shaderFilename.end());
	//std::string beforeExtension = std::string(shaderFilename.begin(), dot);

	//std::string compiledFilename = beforeExtension + "_" + extension + ".spv";

	ShaderComponent component;
	component.filename = getValue(entry.filenameEntry);
	component.type = getValue(entry.stageEntry);

	return component;
}

ShaderEntry submitEntry(EntrySpecification<ShaderEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager)
{
	ShaderEntry entry;

	entry.container = createContainer(guiContext, ContainerSpecification(
		nullGUIParent(),
		ContainerOrdering::VERTICAL,
		OffsetMethod::EXTENT
	));
	entry.base = entry.container.base;

	entry.filenameEntry = submitEntry(EntrySpecification<StringTextEntry>("Enter filename"), guiContext, resourceManager);
	entry.stageEntry = submitEntry(EntrySpecification<ObjectEntry<ShaderStage>>(
		ShaderStage::VERTEX,
		{ ShaderStage::FRAGMENT, ShaderStage::VERTEX }
	), guiContext, resourceManager);

	addChild(entry.container.base, { &entry.filenameEntry.base, 1 }, guiContext);
	addChild(entry.container.base, { &entry.stageEntry.base, 1 }, guiContext);

	return entry;
}

void setupEntry(ShaderEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext)
{
	setupEntry(entry.filenameEntry, manager, engine, context, guiContext);
	setupEntry(entry.stageEntry, manager, engine, context, guiContext);

	properties(entry.filenameEntry.base, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.filenameEntry.base, guiContext).centerY = GUIAnchor::TOP;

	properties(entry.stageEntry.base, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.stageEntry.base, guiContext).centerY = GUIAnchor::TOP;
}

void updateEntry(ShaderEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
{
	updateEntry(entry.filenameEntry, guiContext, engine, context);
	updateEntry(entry.stageEntry, guiContext, engine, context);
}

void submitEntries(std::span<ShaderEntry*> const entries, GUIContext& guiContext)
{
	for (ShaderEntry* entry : entries) {
		StringTextEntry* stringEntries[] = { &entry->filenameEntry };
		ObjectEntry<ShaderStage>* stageEntries[] = { &entry->stageEntry };

		submitEntries(stringEntries, guiContext);
		submitEntries<ShaderStage>(stageEntries, guiContext);
	}
}

void dropEntry(ShaderEntry& entry, Engine& engine, GUIContext& guiContext)
{
	dropEntry(entry.filenameEntry, engine, guiContext);
	dropEntry(entry.stageEntry, engine, guiContext);
}

DescriptorLayoutComponent getValue(DescriptorLayoutEntry const& entry)
{
	DescriptorLayoutComponent component;
	component.bindings = getValue(entry.bindingEntries);

	return component;
}

DescriptorLayoutEntry submitEntry(EntrySpecification<DescriptorLayoutEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager)
{
	DescriptorLayoutEntry entry;

	entry.entrySpec = new EntrySpecification<UniformBindingEntry>();
	entry.bindingEntries = submitEntry(EntrySpecification<ListEntry<UniformBindingEntry>>(
		5,
		entry.entrySpec
	), guiContext, resourceManager);
	
	entry.base = entry.bindingEntries.base;

	return entry;
}

void setupEntry(DescriptorLayoutEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext)
{
	setupEntry(entry.bindingEntries, manager, engine, context, guiContext);
}

void updateEntry(DescriptorLayoutEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
{
	updateEntry(entry.bindingEntries, guiContext, engine, context);
}

void submitEntries(std::span<DescriptorLayoutEntry*> const entries, GUIContext& guiContext)
{
	for (DescriptorLayoutEntry* entry : entries) {
		ListEntry<UniformBindingEntry>* uniforms[] = {&entry->bindingEntries};

		submitEntries<UniformBindingEntry>(uniforms, guiContext);
	}
}

void dropEntry(DescriptorLayoutEntry& entry, Engine& engine, GUIContext& guiContext)
{
	delete entry.entrySpec;

	dropEntry(entry.bindingEntries, engine, guiContext);
}

BufferComponent getValue(BufferEntry const& entry)
{
	BufferComponent component;

	// TODO: better way to copy vectors like this?
	std::vector<float> floatData = getValue(entry.data);

	component.data.resize(sizeof(float) * floatData.size());
	memcpy(component.data.data(), floatData.data(), component.data.size());

	component.numElements = floatData.size();
	component.usage = getValue(entry.usage);

	return component;
}

BufferEntry submitEntry(EntrySpecification<BufferEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager)
{
	BufferEntry entry;

	entry.entrySpec = new EntrySpecification<FloatTextEntry>();
	entry.entrySpec->placeholder = "Value";

	entry.container = createContainer(guiContext, ContainerSpecification(
		nullGUIParent(),
		ContainerOrdering::VERTICAL,
		OffsetMethod::EXTENT
	));
	entry.base = entry.container.base;

	entry.usage = submitEntry(EntrySpecification<ObjectEntry<BufferUsage>>(
		BufferUsage::VERTEX,
		std::vector<BufferUsage>({
			BufferUsage::INDEX,
			BufferUsage::VERTEX,
			BufferUsage::STORAGE,
			BufferUsage::UNIFORM
		})
	), guiContext, resourceManager);

	entry.data = submitEntry(EntrySpecification<ListEntry<FloatTextEntry>>(
		16,
		entry.entrySpec
	), guiContext, resourceManager);

	addChild(entry.container.base, { &entry.usage.base, 1 }, guiContext);
	addChild(entry.container.base, { &entry.data.base, 1 }, guiContext);
	
	return entry;
}

void setupEntry(BufferEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext)
{
	setupEntry(entry.usage, manager, engine, context, guiContext);
	setupEntry(entry.data, manager, engine, context, guiContext);

	properties(entry.usage.base, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.data.base, guiContext).anchorY = GUIAnchor::TOP;

	properties(entry.usage.base, guiContext).centerY = GUIAnchor::TOP;
	properties(entry.data.base, guiContext).centerY = GUIAnchor::TOP;
}

void updateEntry(BufferEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
{
	updateEntry(entry.data, guiContext, engine, context);
	updateEntry(entry.usage, guiContext, engine, context);
}

void submitEntries(std::span<BufferEntry*> const entries, GUIContext& guiContext)
{
	for (BufferEntry* entry : entries) {
		ListEntry<FloatTextEntry>* dataEntries[] = { &entry->data };
		ObjectEntry<BufferUsage>* usageEntries[] = { &entry->usage };

		submitEntries<FloatTextEntry>(dataEntries, guiContext);
		submitEntries<BufferUsage>(usageEntries, guiContext);

		GUIProperties& propsUsage = properties(entry->usage.base, guiContext);
		debugRect(
			propsUsage.minExtent,
			propsUsage.maxExtent - propsUsage.minExtent,
			Color(0.0f, 1.0f, 0.0f),
			guiContext
		);

		GUIProperties& propsData = properties(entry->data.base, guiContext);
		debugRect(
			propsData.minExtent,
			propsData.maxExtent - propsData.minExtent,
			Color(1.0f, 0.0f, 1.0f),
			guiContext
		);
	}
}

void dropEntry(BufferEntry& entry, Engine& engine, GUIContext& guiContext)
{
	delete entry.entrySpec;

	dropEntry(entry.data, engine, guiContext);
	dropEntry(entry.usage, engine, guiContext);
}

PipelineComponent getValue(PipelineEntry const& entry)
{
	PipelineComponent pipeline;

	if (entry.bufferLayout.hasValue)
		pipeline.bufferLayout = getValue(entry.bufferLayout);
	if (entry.descriptorLayout.hasValue)
		pipeline.descriptorLayout = getValue(entry.descriptorLayout);
	if (entry.vertexShader.hasValue)
		pipeline.vertexShader = getValue(entry.vertexShader);
	if (entry.fragmentShader.hasValue)
		pipeline.fragmentShader = getValue(entry.fragmentShader);
	if (entry.vertexBuffer.hasValue)
		pipeline.vertexBuffer = getValue(entry.vertexBuffer);
	if (entry.indexBuffer.hasValue)
		pipeline.indexBuffer = getValue(entry.indexBuffer);
	if (entry.descriptorSet.hasValue)
		pipeline.descriptorSet = getValue(entry.descriptorSet);

	return pipeline;
}

PipelineEntry submitEntry(EntrySpecification<PipelineEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager)
{
	PipelineEntry entry;
	entry.registry = spec.registry;

	entry.container = createContainer(guiContext, ContainerSpecification(
		nullGUIParent(),
		ContainerOrdering::VERTICAL,
		OffsetMethod::EXTENT
	));
	entry.base = entry.container.base;

	entry.bufferLayout = submitEntry(EntrySpecification<UploadEntry<Entity>>("Buffer layout", spec.heldItemPointer), guiContext, resourceManager);
	entry.descriptorLayout = submitEntry(EntrySpecification<UploadEntry<Entity>>("Descriptor layout", spec.heldItemPointer), guiContext, resourceManager);
	entry.vertexShader = submitEntry(EntrySpecification<UploadEntry<Entity>>("Vertex shader", spec.heldItemPointer), guiContext, resourceManager);
	entry.fragmentShader = submitEntry(EntrySpecification<UploadEntry<Entity>>("Fragment shader", spec.heldItemPointer), guiContext, resourceManager);
	entry.vertexBuffer = submitEntry(EntrySpecification<UploadEntry<Entity>>("Vertex buffer", spec.heldItemPointer), guiContext, resourceManager);
	entry.indexBuffer = submitEntry(EntrySpecification<UploadEntry<Entity>>("Index buffer", spec.heldItemPointer), guiContext, resourceManager);
	entry.descriptorSet = submitEntry(EntrySpecification<UploadEntry<Entity>>("Descriptor set", spec.heldItemPointer), guiContext, resourceManager);

	addChild(entry.container.base, { &entry.bufferLayout.base, 1 }, guiContext);
	addChild(entry.container.base, { &entry.descriptorLayout.base, 1 }, guiContext);
	addChild(entry.container.base, { &entry.vertexShader.base, 1 }, guiContext);
	addChild(entry.container.base, { &entry.fragmentShader.base, 1 }, guiContext);
	addChild(entry.container.base, { &entry.vertexBuffer.base, 1 }, guiContext);
	addChild(entry.container.base, { &entry.indexBuffer.base, 1 }, guiContext);
	addChild(entry.container.base, { &entry.descriptorSet.base, 1 }, guiContext);

	return entry;
}

void setupEntry(PipelineEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext)
{
	setupEntry(entry.bufferLayout, manager, engine, context, guiContext);
	setupEntry(entry.descriptorLayout, manager, engine, context, guiContext);
	setupEntry(entry.vertexShader, manager, engine, context, guiContext);
	setupEntry(entry.fragmentShader, manager, engine, context, guiContext);
	setupEntry(entry.vertexBuffer, manager, engine, context, guiContext);
	setupEntry(entry.indexBuffer, manager, engine, context, guiContext);
	setupEntry(entry.descriptorSet, manager, engine, context, guiContext);

	properties(entry.bufferLayout, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.descriptorLayout, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.vertexShader, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.fragmentShader, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.vertexBuffer, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.indexBuffer, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.descriptorSet, guiContext).anchorY = GUIAnchor::TOP;

	properties(entry.bufferLayout, guiContext).centerY = GUIAnchor::TOP;
	properties(entry.descriptorLayout, guiContext).centerY = GUIAnchor::TOP;
	properties(entry.vertexShader, guiContext).centerY = GUIAnchor::TOP;
	properties(entry.fragmentShader, guiContext).centerY = GUIAnchor::TOP;
	properties(entry.vertexBuffer, guiContext).centerY = GUIAnchor::TOP;
	properties(entry.indexBuffer, guiContext).centerY = GUIAnchor::TOP;
	properties(entry.descriptorSet, guiContext).centerY = GUIAnchor::TOP;
}

void updateEntry(PipelineEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
{
	updateEntry(entry.bufferLayout, guiContext, engine, context);
	updateEntry(entry.descriptorLayout, guiContext, engine, context);
	updateEntry(entry.vertexShader, guiContext, engine, context);
	updateEntry(entry.fragmentShader, guiContext, engine, context);
	updateEntry(entry.vertexBuffer, guiContext, engine, context);
	updateEntry(entry.indexBuffer, guiContext, engine, context);
	updateEntry(entry.descriptorSet, guiContext, engine, context);
}

void submitEntries(std::span<PipelineEntry*> const entries, GUIContext& guiContext)
{
	for (PipelineEntry* entry : entries) {
		UploadEntry<Entity>* uploads[] = {
			&entry->bufferLayout,
			&entry->descriptorLayout,
			&entry->vertexShader,
			&entry->fragmentShader,
			&entry->vertexBuffer,
			&entry->indexBuffer,
			&entry->descriptorSet
		};

		submitEntries<Entity>(uploads, guiContext);
	}
}

void dropEntry(PipelineEntry& entry, Engine& engine, GUIContext& guiContext)
{
	dropEntry(entry.bufferLayout, engine, guiContext);
	dropEntry(entry.descriptorLayout, engine, guiContext);
	dropEntry(entry.vertexShader, engine, guiContext);
	dropEntry(entry.fragmentShader, engine, guiContext);
	dropEntry(entry.vertexBuffer, engine, guiContext);
	dropEntry(entry.indexBuffer, engine, guiContext);
	dropEntry(entry.descriptorSet, engine, guiContext);
}

DescriptorSetComponent getValue(DescriptorSetEntry const& entry)
{
	DescriptorSetComponent set;

	std::vector<Entity> entities = getValue(entry.uniformData);

	if (entities.size() == 0) { return set; }

	for (Entity entity : entities) {
		if (entry.registry->hasComponent<BufferComponent>(entity)) {
			set.bindingData.push_back(entity);
		}
		else {
			VIVIUM_LOG(LogSeverity::ERROR, "Passed entity {} to descriptor set, but had no compatible/implemented type", entity);
		}

		// TODO: textures/framebuffers
	}

	return set;
}

DescriptorSetEntry submitEntry(EntrySpecification<DescriptorSetEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager)
{
	DescriptorSetEntry entry;

	entry.entrySpec = new EntrySpecification<UploadEntry<Entity>>();
	entry.entrySpec->placeholder = "Uniform data entity";
	entry.entrySpec->heldItemPointer = spec.heldItemPointer;

	entry.uniformData = submitEntry(EntrySpecification<ListEntry<UploadEntry<Entity>>>(4, entry.entrySpec), guiContext, resourceManager);
	entry.base = entry.uniformData.base;
	entry.registry = spec.registry;

	return entry;
}

void setupEntry(DescriptorSetEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext)
{
	setupEntry(entry.uniformData, manager, engine, context, guiContext);
}

void updateEntry(DescriptorSetEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
{
	updateEntry(entry.uniformData, guiContext, engine, context);
}

void submitEntries(std::span<DescriptorSetEntry*> const entries, GUIContext& guiContext)
{
	for (DescriptorSetEntry* entry : entries) {
		ListEntry<UploadEntry<Entity>>* listEntry[] = { &entry->uniformData };

		submitEntries<UploadEntry<Entity>>(listEntry, guiContext);
	}
}

void dropEntry(DescriptorSetEntry& entry, Engine& engine, GUIContext& guiContext)
{
	delete entry.entrySpec;

	dropEntry(entry.uniformData, engine, guiContext);
}