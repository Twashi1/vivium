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
	DESCRIPTOR_SET
};

std::string getString(VulkanComponent component);

struct PipelineComponent {
	Entity bufferLayout;
	Entity descriptorLayout;
	Entity vertexShader;
	Entity fragmentShader;
	
	Entity vertexBuffer;
	Entity indexBuffer;
	Entity descriptorSet;
};

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

struct DescriptorSetComponent {
	std::vector<Entity> bindingData;
};

/*
		typename T::ValueType;
		{ a.base } -> std::same_as<GUIElementReference&>;
		{ getValue(b) } -> std::same_as<typename T::ValueType>;
		{ submitEntry(entrySpec, guiContext, resourceManager) } -> std::same_as<T>;
		{ setupEntry(a, resourceManager, engine, commandContext, guiContext) } -> std::same_as<void>;
		{ updateEntry(a, guiContext, engine, commandContext) } -> std::same_as<void>;
		{ submitEntries(span, guiContext) } -> std::same_as<void>;
		{ dropEntry(a, engine, guiContext) } -> std::same_as<void>;
*/

struct PipelineEntry {
	using ValueType = PipelineComponent;

	GUIElementReference base;
	// TODO: need the drag and drop for entities or something of sort

	UploadEntry<Entity> vertexShader;
	UploadEntry<Entity> fragmentShader;
};

struct ShaderEntry {
	using ValueType = ShaderComponent;

	GUIElementReference base;

	StringTextEntry filenameEntry;
	ObjectEntry<ShaderStage> stageEntry;
};

struct BufferLayoutEntry {
	using ValueType = BufferLayoutComponent;

	GUIElementReference base;

	ListEntry<ObjectEntry<ShaderDataType>> typesEntry;
};

// TODO: have to design this to fit the specifications of an entry
struct UniformBindingEntry {
	using ValueType = UniformBinding;

	GUIElementReference base;

	Panel background;
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

	ObjectEntry<BufferUsage> usage;
	ListEntry<FloatTextEntry> data;
	EntrySpecification<FloatTextEntry>* entrySpec;
};

// TODO: should be union really but silly default constructor (let me be bad)
struct DescriptorSetComponentItem {
	BufferComponent buffer;
};

template <>
struct EntrySpecification<UniformBindingEntry> {};

template <>
struct EntrySpecification<BufferLayoutEntry> {};

template <>
struct EntrySpecification<ShaderEntry> {};

template <>
struct EntrySpecification<PipelineEntry> {};

template <>
struct EntrySpecification<DescriptorLayoutEntry> {};

template <>
struct EntrySpecification<BufferEntry> {};

UniformBinding getValue(UniformBindingEntry& entry);
UniformBindingEntry submitEntry(EntrySpecification<UniformBindingEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(UniformBindingEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(UniformBindingEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<UniformBindingEntry*> const entries, GUIContext& guiContext);
void dropEntry(UniformBindingEntry& entry, Engine& engine, GUIContext& guiContext);

BufferLayoutComponent getValue(BufferLayoutEntry& entry);
BufferLayoutEntry submitEntry(EntrySpecification<BufferLayoutEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(BufferLayoutEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(BufferLayoutEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<BufferLayoutEntry*> const entries, GUIContext& guiContext);
void dropEntry(BufferLayoutEntry& entry, Engine& engine, GUIContext& guiContext);

ShaderComponent getValue(ShaderEntry& entry);
ShaderEntry submitEntry(EntrySpecification<ShaderEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(ShaderEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(ShaderEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<ShaderEntry*> const entries, GUIContext& guiContext);
void dropEntry(ShaderEntry& entry, Engine& engine, GUIContext& guiContext);

DescriptorLayoutComponent getValue(DescriptorLayoutEntry& entry);
DescriptorLayoutEntry submitEntry(EntrySpecification<DescriptorLayoutEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(DescriptorLayoutEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(DescriptorLayoutEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<DescriptorLayoutEntry*> const entries, GUIContext& guiContext);
void dropEntry(DescriptorLayoutEntry& entry, Engine& engine, GUIContext& guiContext);

BufferComponent getValue(BufferEntry& entry);
BufferEntry submitEntry(EntrySpecification<BufferEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(BufferEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(BufferEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<BufferEntry*> const entries, GUIContext& guiContext);
void dropEntry(BufferEntry& entry, Engine& engine, GUIContext& guiContext);

/*
- click on an entity
- want to open inspector window
- click add button
-	object entry, component type
- adds a component
-	grab relevant entry
-	organises into a vbox (... problem, we can't create new entries at runtime)
-	if we add components that look for certain children?
-	component is added to the entity
-	upon updating the entity (through tree container?)
-		component that requires children of certain type
-		look through all children entities
-		any with relevant component are then tracked and added
-		and we fill out the component's data for that frame


alternatively?
-	just treat the entity tree as an organisation tool, not relevant to representing hierarchys
-	and instead we use upload entries (much easier... better?)
*/

std::vector<DescriptorSetComponentItem> findEntitiesForDescriptorSet(std::span<Entity> entities, Registry registry);
void fillOutPipeline(std::span<Entity> entities, PipelineComponent& pipeline, Registry registry);