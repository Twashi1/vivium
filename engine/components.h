#pragma once

#include "engine.h"

/*
Perspective perspective = orthogonalPerspective2D(windowDimensions(window), F32x2(0.0f), 0.0f, 1.0f);

setBuffer(guiContext.button.storageBuffer.resource, 0, guiContext.button.buttons.data(), guiContext.button.buttons.size() * sizeof(_GUIButtonInstanceData));
cmdBindPipeline(context, guiContext.button.pipeline.resource);
cmdBindVertexBuffer(context, guiContext.rectVertexBuffer.resource);
cmdBindIndexBuffer(context, guiContext.rectIndexBuffer.resource);
cmdBindDescriptorSet(context, guiContext.button.descriptorSet.resource, guiContext.button.pipeline.resource);
cmdWritePushConstants(context, &perspective, sizeof(Perspective), 0, ShaderStage::VERTEX, guiContext.button.pipeline.resource);
cmdDrawIndexed(context, 6, guiContext.button.buttons.size());

guiContext.button.buttons.clear();
*/

// TODO: we'll do perspective for them, the push constants, and the draw command
// bind pipeline
// bind vertex buffer
// bind index buffer
// bind descriptor set

// for definition of pipeline
//	need specification for descriptor layout
//	need specification for buffer layout
//  need shaders

// 1. we add components to entities (need button/selection)
//	these components come with entries, which we fill out
//  some components take their data from children of the entity they have been placed on

// 2. parse the tree, looking for each entity with pipeline components (or just go through registry)
//	resolve all dependencies
//	compile to intermediary
//	interpret intermediary to create vulkan objects

enum VulkanComponent {
	PIPELINE,
	BUFFER_LAYOUT,
	DESCRIPTOR_LAYOUT,
	SHADER,
	BUFFER,
	DESCRIPTOR_SET,
	ENTER_COMPONENT
};

std::string getString(VulkanComponent component);

struct BufferLayoutComponent {
	std::vector<ShaderDataType> types;
};

struct DescriptorLayoutComponent {
	std::vector<UniformBinding> bindings;
};

struct ShaderComponent {
	std::string filename;
	ShaderStage type;
};

struct BufferComponent {
	std::vector<float> data;
	BufferUsage usage;
	uint64_t size;
};

struct DescriptorSetItem {
	BufferComponent bufferPart;
};

struct DescriptorSetComponent {
	std::vector<DescriptorSetItem> bindingData;
};

struct PipelineComponent {
	BufferLayoutComponent bufferLayout;
	DescriptorLayoutComponent descriptorLayout;
	ShaderComponent vertexShader;
	ShaderComponent fragmentShader;

	BufferComponent vertexBuffer;
	BufferComponent indexBuffer;
	DescriptorSetComponent descriptorSet;
};

struct PipelineEntry {
	using ValueType = PipelineComponent;

	GUIElementReference base;

	Container container;
	UploadEntry<Entity> bufferLayout;
	UploadEntry<Entity> descriptorLayout;
	UploadEntry<Entity> vertexShader;
	UploadEntry<Entity> fragmentShader;
	UploadEntry<Entity> vertexBuffer;
	UploadEntry<Entity> indexBuffer;
	UploadEntry<Entity> descriptorSet;

	Registry* registry;
};

struct ShaderEntry {
	using ValueType = ShaderComponent;

	GUIElementReference base;

	Container container;
	StringTextEntry filenameEntry;
	ObjectEntry<ShaderStage> stageEntry;
};

struct BufferLayoutEntry {
	using ValueType = BufferLayoutComponent;

	GUIElementReference base;

	ListEntry<ObjectEntry<ShaderDataType>> typesEntry;
};

struct UniformBindingEntry {
	using ValueType = UniformBinding;

	GUIElementReference base;

	Container entryContainer;

	ObjectEntry<ShaderStage> stageEntry;
	IntegerTextEntry slotEntry;
	ObjectEntry<UniformType> typeEntry;
};

struct DescriptorLayoutEntry {
	using ValueType = DescriptorLayoutComponent;

	GUIElementReference base;

	EntrySpecification<UniformBindingEntry>* entrySpec;
	ListEntry<UniformBindingEntry> bindingEntries;
};

struct BufferEntry {
	using ValueType = BufferComponent;

	GUIElementReference base;

	Container container;
	ObjectEntry<BufferUsage> usage;
	ListEntry<FloatTextEntry> data;
	EntrySpecification<FloatTextEntry>* entrySpec;
};

struct DescriptorSetEntry {
	using ValueType = DescriptorSetComponent;

	GUIElementReference base;

	Registry* registry;
	ListEntry<UploadEntry<Entity>> uniformData;
	EntrySpecification<UploadEntry<Entity>>* entrySpec;
};

template <>
struct EntrySpecification<UniformBindingEntry> {};

template <>
struct EntrySpecification<BufferLayoutEntry> {};

template <>
struct EntrySpecification<ShaderEntry> {};

template <>
struct EntrySpecification<PipelineEntry> {
	Registry* registry;
	Entity** heldItemPointer;
};

template <>
struct EntrySpecification<DescriptorLayoutEntry> {};

template <>
struct EntrySpecification<BufferEntry> {};

template <>
struct EntrySpecification<DescriptorSetEntry> {
	Registry* registry;
	Entity** heldItemPointer;
};

UniformBinding getValue(UniformBindingEntry const& entry);
UniformBindingEntry submitEntry(EntrySpecification<UniformBindingEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(UniformBindingEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(UniformBindingEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<UniformBindingEntry*> const entries, GUIContext& guiContext);
void dropEntry(UniformBindingEntry& entry, Engine& engine, GUIContext& guiContext);

BufferLayoutComponent getValue(BufferLayoutEntry const& entry);
BufferLayoutEntry submitEntry(EntrySpecification<BufferLayoutEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(BufferLayoutEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(BufferLayoutEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<BufferLayoutEntry*> const entries, GUIContext& guiContext);
void dropEntry(BufferLayoutEntry& entry, Engine& engine, GUIContext& guiContext);

ShaderComponent getValue(ShaderEntry const& entry);
ShaderEntry submitEntry(EntrySpecification<ShaderEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(ShaderEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(ShaderEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<ShaderEntry*> const entries, GUIContext& guiContext);
void dropEntry(ShaderEntry& entry, Engine& engine, GUIContext& guiContext);

DescriptorLayoutComponent getValue(DescriptorLayoutEntry const& entry);
DescriptorLayoutEntry submitEntry(EntrySpecification<DescriptorLayoutEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(DescriptorLayoutEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(DescriptorLayoutEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<DescriptorLayoutEntry*> const entries, GUIContext& guiContext);
void dropEntry(DescriptorLayoutEntry& entry, Engine& engine, GUIContext& guiContext);

BufferComponent getValue(BufferEntry const& entry);
BufferEntry submitEntry(EntrySpecification<BufferEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(BufferEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(BufferEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<BufferEntry*> const entries, GUIContext& guiContext);
void dropEntry(BufferEntry& entry, Engine& engine, GUIContext& guiContext);

PipelineComponent getValue(PipelineEntry const& entry);
PipelineEntry submitEntry(EntrySpecification<PipelineEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(PipelineEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(PipelineEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<PipelineEntry*> const entries, GUIContext& guiContext);
void dropEntry(PipelineEntry& entry, Engine& engine, GUIContext& guiContext);

DescriptorSetComponent getValue(DescriptorSetEntry const& entry);
DescriptorSetEntry submitEntry(EntrySpecification<DescriptorSetEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(DescriptorSetEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(DescriptorSetEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<DescriptorSetEntry*> const entries, GUIContext& guiContext);
void dropEntry(DescriptorSetEntry& entry, Engine& engine, GUIContext& guiContext);

struct PipelineBlueprint {
	uint32_t bufferLayout;
	uint32_t descriptorLayout;
	uint32_t vertexShader;
	uint32_t fragmentShader;
	uint32_t vertexBuffer;
	uint32_t indexBuffer;
	uint32_t descriptorSet;
};

struct DescriptorSetBlueprint {
	std::vector<uint32_t> bindingReferences;
};

struct ComponentHeaderBlueprint {
	VulkanComponent component;
	uint32_t index;
};

template <SerialiserInterface T>
void serialiseWrite(ComponentHeaderBlueprint const& blueprint, T& store) {
	serialiseWrite(blueprint.component, store);
	serialiseWrite(blueprint.index, store);
}

template <SerialiserInterface T>
void serialiseRead(ComponentHeaderBlueprint* blueprint, T& store) {
	serialiseRead(&blueprint->component, store);
	serialiseRead(&blueprint->index, store);
}

template <SerialiserInterface T>
uint32_t writeComponent(BufferComponent const& component, T& store, uint32_t& objectReference)
{
	ComponentHeaderBlueprint head;
	head.component = VulkanComponent::BUFFER;
	head.index = objectReference++;

	serialiseWrite(head, store);

	serialiseWrite(component.data, store);
	serialiseWrite(component.usage, store);
	// TODO: size not required...
	serialiseWrite(component.size, store);

	return head.index;
}

template <SerialiserInterface T>
uint32_t writeComponent(ShaderComponent const& component, T& store, uint32_t& objectReference)
{
	ComponentHeaderBlueprint head;
	head.component = VulkanComponent::SHADER;
	head.index = objectReference++;

	serialiseWrite(head, store);

	serialiseWrite(component.filename, store);
	serialiseWrite(component.type, store);

	return head.index;
}

template <SerialiserInterface T>
uint32_t writeComponent(BufferLayoutComponent const& component, T& store, uint32_t& objectReference)
{
	ComponentHeaderBlueprint head;
	head.component = VulkanComponent::BUFFER_LAYOUT;
	head.index = objectReference++;

	serialiseWrite(head, store);

	serialiseWrite(component.types, store);

	return head.index;
}

template <SerialiserInterface T>
uint32_t writeComponent(DescriptorLayoutComponent const& component, T& store, uint32_t& objectReference)
{
	ComponentHeaderBlueprint head;
	head.component = VulkanComponent::DESCRIPTOR_LAYOUT;
	head.index = objectReference++;

	serialiseWrite(head, store);

	serialiseWrite(component.bindings, store);

	return head.index;
}

template <SerialiserInterface T>
uint32_t writeComponent(DescriptorSetComponent const& component, T& store, uint32_t& objectReference)
{
	ComponentHeaderBlueprint head;
	head.component = VulkanComponent::DESCRIPTOR_SET;
	head.index = objectReference++;

	serialiseWrite(head, store);

	serialiseWrite(component.bindingData.size(), store);

	DescriptorSetBlueprint blueprint;
	blueprint.bindingReferences.reserve(component.bindingData.size());

	for (DescriptorSetItem const& item : component.bindingData) {
		// TODO: without the layout, we can't tell what's stored at each item,
		//	for now we only have one itme type so we can just assume that, but this will need big changes
		//	in future with image/framebuffer support
		uint32_t itemReference = writeComponent(item.bufferPart, store, objectReference);
		blueprint.bindingReferences.push_back(itemReference);
	}

	return head.index;
}

template <SerialiserInterface T>
uint32_t writeComponent(PipelineComponent const& component, T& store, uint32_t& objectReference)
{
	ComponentHeaderBlueprint head;
	head.component = VulkanComponent::PIPELINE;
	head.index = objectReference++;

	serialiseWrite(head, store);

	PipelineBlueprint blueprint;
	blueprint.vertexBuffer = writeComponent(component.vertexBuffer, store, objectReference);
	blueprint.indexBuffer = writeComponent(component.indexBuffer, store, objectReference);

	blueprint.fragmentShader = writeComponent(component.fragmentShader, store, objectReference);
	blueprint.vertexShader = writeComponent(component.vertexShader, store, objectReference);
	
	blueprint.bufferLayout = writeComponent(component.bufferLayout, store, objectReference);
	blueprint.descriptorLayout = writeComponent(component.descriptorLayout, store, objectReference);

	blueprint.descriptorSet = writeComponent(component.descriptorSet, store, objectReference);
	
	return head.index;
}

// TODO; read counterparts