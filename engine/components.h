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

struct PipelineComponent {
	Entity bufferLayout;
	Entity descriptorLayout;
	Entity vertexShader;
	Entity fragmentShader;
	Entity vertexBuffer;
	Entity indexBuffer;
	Entity descriptorSet;
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

struct ShaderComponent {
	std::string filename;
	ShaderStage type;
};

struct ShaderEntry {
	using ValueType = ShaderComponent;

	GUIElementReference base;

	Container container;
	StringTextEntry filenameEntry;
	ObjectEntry<ShaderStage> stageEntry;
};

struct BufferLayoutComponent {
	std::vector<ShaderDataType> types;
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

struct DescriptorLayoutComponent {
	std::vector<UniformBinding> bindings;
};

struct DescriptorLayoutEntry {
	using ValueType = DescriptorLayoutComponent;

	GUIElementReference base;

	EntrySpecification<UniformBindingEntry>* entrySpec;
	ListEntry<UniformBindingEntry> bindingEntries;
};

struct BufferComponent {
	std::vector<uint8_t> data;
	BufferUsage usage;
	uint64_t numElements;
};

struct BufferEntry {
	using ValueType = BufferComponent;

	GUIElementReference base;

	Container container;
	ObjectEntry<BufferUsage> usage;
	ListEntry<FloatTextEntry> data;
	EntrySpecification<FloatTextEntry>* entrySpec;
};

struct DescriptorSetComponent {
	std::vector<Entity> bindingData;
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
void loadValue(UniformBindingEntry& entry, UniformBinding const& binding, GUIContext& guiContext);

BufferLayoutComponent getValue(BufferLayoutEntry const& entry);
BufferLayoutEntry submitEntry(EntrySpecification<BufferLayoutEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(BufferLayoutEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(BufferLayoutEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<BufferLayoutEntry*> const entries, GUIContext& guiContext);
void dropEntry(BufferLayoutEntry& entry, Engine& engine, GUIContext& guiContext);
void loadValue(BufferLayoutEntry& entry, BufferLayoutComponent const& layout, GUIContext& guiContext);

ShaderComponent getValue(ShaderEntry const& entry);
ShaderEntry submitEntry(EntrySpecification<ShaderEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(ShaderEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(ShaderEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<ShaderEntry*> const entries, GUIContext& guiContext);
void dropEntry(ShaderEntry& entry, Engine& engine, GUIContext& guiContext);
void loadValue(ShaderEntry& entry, ShaderComponent const& shader, GUIContext& guiContext);

DescriptorLayoutComponent getValue(DescriptorLayoutEntry const& entry);
DescriptorLayoutEntry submitEntry(EntrySpecification<DescriptorLayoutEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(DescriptorLayoutEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(DescriptorLayoutEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<DescriptorLayoutEntry*> const entries, GUIContext& guiContext);
void dropEntry(DescriptorLayoutEntry& entry, Engine& engine, GUIContext& guiContext);
void loadValue(DescriptorLayoutEntry& entry, DescriptorLayoutComponent const& descriptorLayout, GUIContext& guiContext);

BufferComponent getValue(BufferEntry const& entry);
BufferEntry submitEntry(EntrySpecification<BufferEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(BufferEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(BufferEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<BufferEntry*> const entries, GUIContext& guiContext);
void dropEntry(BufferEntry& entry, Engine& engine, GUIContext& guiContext);
void loadValue(BufferEntry& entry, BufferComponent const& buffer, GUIContext& guiContext);

PipelineComponent getValue(PipelineEntry const& entry);
PipelineEntry submitEntry(EntrySpecification<PipelineEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(PipelineEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(PipelineEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<PipelineEntry*> const entries, GUIContext& guiContext);
void dropEntry(PipelineEntry& entry, Engine& engine, GUIContext& guiContext);
void loadValue(PipelineEntry& entry, PipelineComponent const& pipeline, GUIContext& guiContext);

DescriptorSetComponent getValue(DescriptorSetEntry const& entry);
DescriptorSetEntry submitEntry(EntrySpecification<DescriptorSetEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager);
void setupEntry(DescriptorSetEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext);
void updateEntry(DescriptorSetEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
void submitEntries(std::span<DescriptorSetEntry*> const entries, GUIContext& guiContext);
void dropEntry(DescriptorSetEntry& entry, Engine& engine, GUIContext& guiContext);
void loadValue(DescriptorSetEntry& entry, DescriptorSetComponent const& descriptor, GUIContext& guiContext);

struct ComponentHeaderBlueprint {
	VulkanComponent component;
};

namespace Vivium {
	template <SerialiserInterface T>
	void serialiseWrite(ComponentHeaderBlueprint const& blueprint, T& store) {
		serialiseWrite(blueprint.component, store);
	}

	template <SerialiserInterface T>
	void serialiseRead(ComponentHeaderBlueprint* blueprint, T& store) {
		serialiseRead(&blueprint->component, store);
	}

	template <SerialiserInterface T>
	void serialiseWrite(BufferComponent const& component, T& store)
	{
		// TODO: remove component header sanity checks?
		ComponentHeaderBlueprint head;
		head.component = VulkanComponent::BUFFER;

		dispatchSerialiseWrite(head, store);

		dispatchSerialiseWrite(component.data, store);
		dispatchSerialiseWrite(component.usage, store);
		dispatchSerialiseWrite(component.numElements, store);
	}

	template <SerialiserInterface T>
	void serialiseWrite(ShaderComponent const& component, T& store)
	{
		ComponentHeaderBlueprint head;
		head.component = VulkanComponent::SHADER;

		dispatchSerialiseWrite(head, store);

		dispatchSerialiseWrite(component.filename, store);
		dispatchSerialiseWrite(component.type, store);
	}

	template <SerialiserInterface T>
	void serialiseWrite(BufferLayoutComponent const& component, T& store)
	{
		ComponentHeaderBlueprint head;
		head.component = VulkanComponent::BUFFER_LAYOUT;

		dispatchSerialiseWrite(head, store);

		dispatchSerialiseWrite(component.types, store);
	}

	template <SerialiserInterface T>
	void serialiseWrite(DescriptorLayoutComponent const& component, T& store)
	{
		ComponentHeaderBlueprint head;
		head.component = VulkanComponent::DESCRIPTOR_LAYOUT;

		dispatchSerialiseWrite(head, store);

		dispatchSerialiseWrite(component.bindings, store);
	}

	template <SerialiserInterface T>
	void serialiseWrite(DescriptorSetComponent const& component, T& store)
	{
		ComponentHeaderBlueprint head;
		head.component = VulkanComponent::DESCRIPTOR_SET;

		dispatchSerialiseWrite(head, store);
		dispatchSerialiseWrite(component.bindingData, store);
	}

	template <SerialiserInterface T>
	void serialiseWrite(PipelineComponent const& component, T& store)
	{
		ComponentHeaderBlueprint head;
		head.component = VulkanComponent::PIPELINE;

		dispatchSerialiseWrite(head, store);

		dispatchSerialiseWrite(component.vertexBuffer, store);
		dispatchSerialiseWrite(component.indexBuffer, store);
		dispatchSerialiseWrite(component.fragmentShader, store);
		dispatchSerialiseWrite(component.vertexShader, store);
		dispatchSerialiseWrite(component.bufferLayout, store);
		dispatchSerialiseWrite(component.descriptorLayout, store);
		dispatchSerialiseWrite(component.descriptorSet, store);
	}

	template <SerialiserInterface T>
	void serialiseRead(BufferComponent* component, T& store)
	{
		ComponentHeaderBlueprint head;

		dispatchSerialiseRead(&head, store);
		// TODO: enum strings for loggin
		VIVIUM_ASSERT(head.component == VulkanComponent::BUFFER, "Read incorrect component type");

		dispatchSerialiseRead(&component->data, store);
		dispatchSerialiseRead(&component->usage, store);
		dispatchSerialiseRead(&component->numElements, store);
	}

	template <SerialiserInterface T>
	void serialiseRead(ShaderComponent* component, T& store)
	{
		ComponentHeaderBlueprint head;

		dispatchSerialiseRead(&head, store);
		// TODO: enum strings for logging
		VIVIUM_ASSERT(head.component == VulkanComponent::SHADER, "Read incorrect component type");

		dispatchSerialiseRead(&component->filename, store);
		dispatchSerialiseRead(&component->type, store);
	}

	template <SerialiserInterface T>
	void serialiseRead(BufferLayoutComponent* component, T& store)
	{
		ComponentHeaderBlueprint head;

		dispatchSerialiseRead(&head, store);
		// TODO: enum strings for logging
		VIVIUM_ASSERT(head.component == VulkanComponent::BUFFER_LAYOUT, "Read incorrect component type");

		dispatchSerialiseRead(&component->types, store);
	}


	template <SerialiserInterface T>
	void serialiseRead(DescriptorLayoutComponent* component, T& store)
	{
		ComponentHeaderBlueprint head;

		dispatchSerialiseRead(&head, store);
		// TODO: enum strings for logging
		VIVIUM_ASSERT(head.component == VulkanComponent::DESCRIPTOR_LAYOUT, "Read incorrect component type");

		dispatchSerialiseRead(&component->bindings, store);
	}

	template <SerialiserInterface T>
	void serialiseRead(DescriptorSetComponent* component, T& store)
	{
		ComponentHeaderBlueprint head;

		dispatchSerialiseRead(&head, store);
		// TODO: enum strings for logging
		VIVIUM_ASSERT(head.component == VulkanComponent::DESCRIPTOR_SET, "Read incorrect component type");

		dispatchSerialiseRead(&component->bindingData, store);
	}

	template <SerialiserInterface T>
	void serialiseRead(PipelineComponent* component, T& store)
	{
		ComponentHeaderBlueprint head;

		dispatchSerialiseRead(&head, store);
		// TODO: enum strings for logging
		VIVIUM_ASSERT(head.component == VulkanComponent::PIPELINE, "Read incorrect component type");

		dispatchSerialiseRead(&component->vertexBuffer, store);
		dispatchSerialiseRead(&component->indexBuffer, store);
		dispatchSerialiseRead(&component->fragmentShader, store);
		dispatchSerialiseRead(&component->vertexShader, store);
		dispatchSerialiseRead(&component->bufferLayout, store);
		dispatchSerialiseRead(&component->descriptorLayout, store);
		dispatchSerialiseRead(&component->descriptorSet, store);
	}
}