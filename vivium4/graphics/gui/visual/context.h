#pragma once

#include "../../../storage.h"
#include "../../resource_manager.h"
#include "../../color.h"
#include "../base.h"
#include "../../../math/atlas.h"

namespace Vivium {
	struct _GUIButtonInstanceData {
		F32x2 position;			// 8 bytes
		F32x2 scale;			// 16 bytes
		Color foregroundColor;	// 28 bytes
		float _fill0;			// 32 bytes
	};

	// TODO: border size not in px
	struct _GUIPanelInstanceData {
		F32x2 position;
		F32x2 scale;
		Color backgroundColor;
		float borderSizePx;		// 32 bytes
		Color borderColor;		// 44 bytes
		float _fill0;			// 48 bytes
	};

	struct _GUISliderInstanceData {
		F32x2 position;			// 8
		F32x2 scale;			// 16
		Color foregroundColor;	// 28
		float percent;			// 32
		Color sliderColor;		// 44
		float contrast;			// 48
		F32x2 sliderScale;		// 56
		float selectorScale;	// 60
		float _fill0;			// 64
	};

	struct _GUISpriteInstanceData {
		F32x2 position;
		F32x2 scale;
		F32x2 texturePosition;
		F32x2 textureScale;
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

		struct {
			static constexpr uint64_t MAX_SLIDERS = 128;

			Ref<Shader> fragmentShader;
			Ref<Shader> vertexShader;

			Ref<Buffer> storageBuffer;

			Ref<DescriptorLayout> descriptorLayout;
			Ref<DescriptorSet> descriptorSet;
			Ref<Pipeline> pipeline;
		} slider;

		struct {
			static constexpr uint64_t MAX_SPRITES = 128;

			Ref<Shader> fragmentShader;
			Ref<Shader> vertexShader;

			Ref<Buffer> storageBuffer;
			Ref<Texture> texture;

			Ref<DescriptorLayout> descriptorLayout;
			Ref<DescriptorSet> descriptorSet;
			Ref<Pipeline> pipeline;

			StitchedAtlas const* atlas;
		} sprite;

		GUIElementReference defaultParent;
		std::vector<GUIElement> guiElements;
	};

	GUIElementReference createGUIElement(GUIContext& context, GUIElementType type);
	GUIElementReference defaultGUIParent(GUIContext& context);

	void _submitGenericGUIContext(GUIContext& guiContext, ResourceManager& manager, Engine& engine, Window& window);
	void _submitTextGUIContext(GUIContext& guiContext, ResourceManager& manager, Engine& engine, Window& window);
	void _submitButtonGUIContext(GUIContext& guiContext, ResourceManager& manager, Engine& engine, Window& window);
	void _submitPanelGUIContext(GUIContext& guiContext, ResourceManager& manager, Engine& engine, Window& window);
	void _submitSliderGUIContext(GUIContext& guiContext, ResourceManager& manager, Engine& engine, Window& window);
	void _submitSpriteGUIContext(GUIContext& guiContext, ResourceManager& manager, Engine& engine, Window& window);
	
	GUIContext createGUIContext(ResourceManager& manager, Engine& engine, Window& window, StitchedAtlas const* spriteAtlas);
				
	void setupGUIContext(GUIContext& guiContext, ResourceManager& manager, CommandContext& context, Engine& engine);
	void updateGUIContext(GUIContext& guiContext, F32x2 windowDimensions);
	void dropGUIContext(GUIContext& guiContext, Engine& engine);
}