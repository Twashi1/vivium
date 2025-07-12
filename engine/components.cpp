#include "components.h"

std::string getString(VulkanComponent component) {
	switch (component) {
	case VulkanComponent::PIPELINE: return "Pipeline";
	case VulkanComponent::BUFFER_LAYOUT: return "Buffer Layout";
	case VulkanComponent::DESCRIPTOR_LAYOUT: return "Descriptor Layout";
	case VulkanComponent::SHADER: return "Shader";
	case VulkanComponent::BUFFER: return "Buffer";
	case VulkanComponent::DESCRIPTOR_SET: return "Descriptor Set";
	default: return "Unknown";
	}
}

UniformBinding getValue(UniformBindingEntry& entry)
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

	entry.base = createGUIElement(guiContext, GUIElementType::ENTRY);
	entry.slotEntry = submitEntry(EntrySpecification<IntegerTextEntry>("Enter slot"), guiContext, resourceManager);

	entry.stageEntry = submitEntry(EntrySpecification<ObjectEntry<ShaderStage>>(
		ShaderStage::VERTEX,
		std::vector<ShaderStage>({ ShaderStage::VERTEX, ShaderStage::FRAGMENT })
	), guiContext, resourceManager);

	entry.typeEntry = submitEntry(EntrySpecification<ObjectEntry<UniformType>>(
		UniformType::STORAGE_BUFFER,
		std::vector<UniformType>({ UniformType::TEXTURE, UniformType::STORAGE_BUFFER, UniformType::UNIFORM_BUFFER })
	), guiContext, resourceManager);

	entry.background = createPanel(guiContext, PanelSpecification(
		entry.base,
		Color(0.1f, 0.1f, 0.1f),
		Color(0.0f, 0.0f, 0.0f),
		0.0f
	));

	entry.entryContainer = createContainer(guiContext, ContainerSpecification(
		entry.base,
		ContainerOrdering::VERTICAL,
		OffsetMethod::EXTENT
	));

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

	properties(entry.background.base, guiContext).dimensions = F32x2(1.0f, 5.0f);
	properties(entry.background.base, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.background.base, guiContext).centerY = GUIAnchor::TOP;

	properties(entry.entryContainer.base, guiContext).dimensions = F32x2(0.9f);
	properties(entry.entryContainer.base, guiContext).position = F32x2(0.0f, -0.1f);
	properties(entry.entryContainer.base, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.entryContainer.base, guiContext).centerY = GUIAnchor::TOP;
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
		Panel* panels[] = { &entry->background };

		submitPanels(panels, guiContext);
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

BufferLayoutComponent getValue(BufferLayoutEntry& entry)
{
	std::vector<ObjectEntry<ShaderDataType>> entryTypes = getValue(entry.typesEntry);
	std::vector<ShaderDataType> types(entryTypes.size());

	for (uint64_t i = 0; i < entryTypes.size(); i++) {
		types[i] = getValue(entryTypes[i]);
	}

	return BufferLayoutComponent(types);
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

ShaderComponent getValue(ShaderEntry& entry)
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

	entry.base = createGUIElement(guiContext, GUIElementType::ENTRY);
	entry.filenameEntry = submitEntry(EntrySpecification<StringTextEntry>("Enter filename"), guiContext, resourceManager);
	entry.stageEntry = submitEntry(EntrySpecification<ObjectEntry<ShaderStage>>(
		ShaderStage::VERTEX,
		{ ShaderStage::FRAGMENT, ShaderStage::VERTEX }
	), guiContext, resourceManager);

	addChild(entry.base, { &entry.filenameEntry.base, 1 }, guiContext);
	addChild(entry.base, { &entry.stageEntry.base, 1 }, guiContext);

	return entry;
}

void setupEntry(ShaderEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext)
{
	setupEntry(entry.filenameEntry, manager, engine, context, guiContext);
	setupEntry(entry.stageEntry, manager, engine, context, guiContext);

	properties(entry.filenameEntry.base, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.filenameEntry.base, guiContext).centerY = GUIAnchor::TOP;
	properties(entry.filenameEntry.base, guiContext).dimensions = F32x2(1.0f, 0.4f);
	properties(entry.stageEntry.base, guiContext).anchorY = GUIAnchor::BOTTOM;
	properties(entry.stageEntry.base, guiContext).centerY = GUIAnchor::BOTTOM;
	properties(entry.stageEntry.base, guiContext).dimensions = F32x2(1.0f, 0.4f);
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

DescriptorLayoutComponent getValue(DescriptorLayoutEntry& entry)
{
	DescriptorLayoutComponent component;
	std::vector<UniformBindingEntry> entries = getValue(entry.bindingEntries);
	component.bindings.resize(entries.size());

	for (uint64_t i = 0; i < component.bindings.size(); i++) {
		component.bindings[i] = getValue(entries[i]);
	}

	return component;
}

DescriptorLayoutEntry submitEntry(EntrySpecification<DescriptorLayoutEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager)
{
	DescriptorLayoutEntry entry;

	entry.base = createGUIElement(guiContext, GUIElementType::ENTRY);
	entry.entrySpec = new EntrySpecification<UniformBindingEntry>();
	entry.bindingEntries = submitEntry(EntrySpecification<ListEntry<UniformBindingEntry>>(
		5,
		entry.entrySpec
	), guiContext, resourceManager);
	
	addChild(entry.base, { &entry.bindingEntries.base, 1 }, guiContext);
}

void setupEntry(DescriptorLayoutEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext)
{
	setupEntry(entry.bindingEntries, manager, engine, context, guiContext);
}

void updateEntry(DescriptorLayoutEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
{
	updateEntry(entry.bindingEntries, guiContext, engine, context);
}

void submitEntries(std::span<DescriptorLayoutEntry*> const entries, GUIContext& guiContext);

void dropEntry(DescriptorLayoutEntry& entry, Engine& engine, GUIContext& guiContext)
{
	delete entry.entrySpec;

	dropEntry(entry.bindingEntries, engine, guiContext);
}

BufferComponent getValue(BufferEntry& entry)
{
	BufferComponent component;

	std::vector<FloatTextEntry> entries = getValue(entry.data);
	component.data.resize(entries.size());

	for (uint64_t i = 0; i < component.data.size(); i++) {
		component.data[i] = getValue(entries[i]);
	}

	component.size = component.data.size() * sizeof(float);
	component.usage = getValue(entry.usage);
}

BufferEntry submitEntry(EntrySpecification<BufferEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager)
{
	BufferEntry entry;

	entry.entrySpec = new EntrySpecification<FloatTextEntry>();
	entry.entrySpec->placeholder = "Value";

	entry.base = createGUIElement(guiContext, GUIElementType::ENTRY);
	entry.usage = submitEntry(EntrySpecification<ObjectEntry<BufferUsage>>(), guiContext, resourceManager);
	entry.data = submitEntry(EntrySpecification<ListEntry<FloatTextEntry>>(
		16,
		entry.entrySpec
	), guiContext, resourceManager);

	addChild(entry.base, { &entry.usage.base, 1 }, guiContext);
	addChild(entry.base, { &entry.data.base, 1 }, guiContext);
}

void setupEntry(BufferEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext)
{
	setupEntry(entry.usage, manager, engine, context, guiContext);
	setupEntry(entry.data, manager, engine, context, guiContext);
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
	}
}

void dropEntry(BufferEntry& entry, Engine& engine, GUIContext& guiContext)
{
	delete entry.entrySpec;
}