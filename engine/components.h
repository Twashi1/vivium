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
	const char* filename;
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

struct PipelineEntry {
	// TODO: need the drag and drop for entities or something of sort
};

struct ShaderEntry {
	StringTextEntry filenameEntry;
	ObjectEntry<ShaderStage> stageEntry;
};

struct BufferLayoutEntry {
	ListEntry<ObjectEntry<ShaderDataType>> typesEntry;
};

// TODO: have to design this to fit the specifications of an entry
struct UniformBindingEntry {
	ObjectEntry<ShaderStage> stageEntry;
	IntegerTextEntry slotEntry;
	ObjectEntry<UniformType> typeEntry;
};