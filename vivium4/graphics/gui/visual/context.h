#pragma once

#include "../../../storage.h"
#include "../../resource_manager.h"
#include "../../color.h"
#include "../base.h"

namespace Vivium {
	struct _GUIButtonInstanceData {
		F32x2 position; // 8 bytes
		F32x2 scale;    // 16 bytes
		Color foregroundColor; // 28 bytes
		float _fill0; // 32 bytes
	};

	// TODO: border size not in px
	struct _GUIPanelInstanceData {
		F32x2 position;
		F32x2 scale;
		Color backgroundColor;
		float borderSizePx; // 32 bytes
		Color borderColor; // 44 bytes
		float _fill0;
	};

	struct GUIContext {
		Ref<Buffer> rectVertexBuffer;
		Ref<Buffer> rectIndexBuffer;

		struct {
			Ref<Pipeline> pipeline;
			Ref<DescriptorLayout> descriptorLayout;
			Ref<Shader> fragmentShader;
			Ref<Shader> vertexShader;

			// Batch buffer layout
			BufferLayout bufferLayout;
		} text;

		struct {
			static constexpr uint64_t MAX_BUTTONS = 128;

			Ref<Shader> fragmentShader;
			Ref<Shader> vertexShader;

			Ref<Buffer> storageBuffer;

			Ref<DescriptorLayout> descriptorLayout;
			Ref<DescriptorSet> descriptorSet;
			Ref<Pipeline> pipeline;
		} button;

		struct {
			static constexpr uint64_t MAX_PANELS = 128;

			Ref<Shader> fragmentShader;
			Ref<Shader> vertexShader;

			Ref<Buffer> storageBuffer;

			Ref<DescriptorLayout> descriptorLayout;
			Ref<DescriptorSet> descriptorSet;
			Ref<Pipeline> pipeline;
		} panel;

		GUIElementReference defaultParent;
		std::vector<GUIElement> guiElements;
	};

	GUIElementReference createGUIElement(GUIContext& context);
	GUIElementReference defaultGUIParent(GUIContext& context);

	void _submitGenericGUIContext(GUIContext& context, ResourceManager& manager, Engine::Handle engine, Window::Handle window);
	void _submitTextGUIContext(GUIContext& context, ResourceManager& manager, Engine::Handle engine, Window::Handle window);
	void _submitButtonGUIContext(GUIContext& context, ResourceManager& manager, Engine::Handle engine, Window::Handle window);
	void _submitPanelGUIContext(GUIContext& context, ResourceManager& manager, Engine::Handle engine, Window::Handle window);

	GUIContext createGUIContext(ResourceManager& manager, Engine::Handle engine, Window::Handle window);
				
	void setupGUIContext(GUIContext& guiContext, ResourceManager& manager, Commands::Context::Handle context, Engine::Handle engine);
	void updateGUIContext(GUIContext& guiContext, F32x2 windowDimensions);
	void dropGUIContext(GUIContext& guiContext, Engine::Handle engine);
}