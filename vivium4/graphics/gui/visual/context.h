#pragma once

#include <array>

#include "../../../math/atlas.h"
#include "../../../storage.h"
#include "../../color.h"
#include "../../resource_manager.h"
#include "../base.h"
#include "text.h"

namespace Vivium {
struct _GUIButtonInstanceData {
  F32x2 position;         // 8 bytes
  F32x2 scale;            // 16 bytes
  Color foregroundColor;  // 28 bytes
  float _fill0;           // 32 bytes
};

// TODO: border size not in px
struct _GUIPanelInstanceData {
  F32x2 position;
  F32x2 scale;
  Color backgroundColor;
  float borderSizePx;  // 32 bytes
  Color borderColor;   // 44 bytes
  float _fill0;        // 48 bytes
};

struct _GUISliderInstanceData {
  F32x2 position;         // 8
  F32x2 scale;            // 16
  Color foregroundColor;  // 28
  float percent;          // 32
  Color sliderColor;      // 44
  float contrast;         // 48
  F32x2 sliderScale;      // 56
  float selectorScale;    // 60
  float _fill0;           // 64
};

struct _GUISpriteInstanceData {
  F32x2 position;
  F32x2 scale;
  F32x2 texturePosition;
  F32x2 textureScale;
};

struct _GUIDebugRectInstanceData {
  F32x2 position;
  F32x2 scale;
  Color borderColor;
  float borderSize;
};

struct GUIContext {
  Ref<Buffer> rectVertexBuffer;
  Ref<Buffer> rectIndexBuffer;

  Font consolas64;
  Ref<Texture> consolas64Texture;

  static constexpr uint64_t MAX_BUTTONS = 128;
  static constexpr uint64_t MAX_PANELS = 128;
  static constexpr uint64_t MAX_SLIDERS = 128;
  static constexpr uint64_t MAX_DEBUG_RECTS = 128;
  static constexpr uint64_t MAX_SPRITES = 128;

  struct {
    Ref<Pipeline> pipeline;
    Ref<DescriptorLayout> descriptorLayout;
    Ref<Shader> fragmentShader;
    Ref<Shader> vertexShader;

    // Batch buffer layout
    BufferLayout bufferLayout;

    std::vector<TextBatch*> texts;
  } text;

  struct {
    Ref<Shader> fragmentShader;
    Ref<Shader> vertexShader;

    Ref<Buffer> storageBuffer;

    Ref<DescriptorLayout> descriptorLayout;
    Ref<DescriptorSet> descriptorSet;
    Ref<Pipeline> pipeline;

    std::vector<_GUIButtonInstanceData> buttons;
  } button;

  struct {
    Ref<Shader> fragmentShader;
    Ref<Shader> vertexShader;

    Ref<Buffer> storageBuffer;

    Ref<DescriptorLayout> descriptorLayout;
    Ref<DescriptorSet> descriptorSet;
    Ref<Pipeline> pipeline;

    std::vector<_GUIPanelInstanceData> panels;
  } panel;

  struct {
    Ref<Shader> fragmentShader;
    Ref<Shader> vertexShader;

    Ref<Buffer> storageBuffer;

    Ref<DescriptorLayout> descriptorLayout;
    Ref<DescriptorSet> descriptorSet;
    Ref<Pipeline> pipeline;

    std::vector<_GUISliderInstanceData> sliders;
  } slider;

  struct {
    Ref<Shader> fragmentShader;
    Ref<Shader> vertexShader;

    Ref<Buffer> storageBuffer;

    Ref<DescriptorLayout> descriptorLayout;
    Ref<DescriptorSet> descriptorSet;
    Ref<Pipeline> pipeline;

    std::vector<_GUIDebugRectInstanceData> rects;
  } debugRect;

  struct {
    Ref<Shader> fragmentShader;
    Ref<Shader> vertexShader;

    Ref<Buffer> storageBuffer;
    Ref<Texture> texture;

    Ref<DescriptorLayout> descriptorLayout;
    Ref<DescriptorSet> descriptorSet;
    Ref<Pipeline> pipeline;

    StitchedAtlas const* atlas;

    std::vector<_GUISpriteInstanceData> sprites;
  } sprite;

  GUIElementReference defaultParent;
  std::vector<GUIElement> guiElements;
};

// TODO: deprecate this method
GUIElementReference createGUIElement(GUIContext& context);
GUIElementReference createGUIElement(GUIContext& context,
                                     GUIElementType elementType);
GUIElementReference createGUIElement(GUIContext& context,
                                     _ContainerUpdateData updateData);
GUIElementReference defaultGUIParent(GUIContext& context);
GUIElementReference nullGUIParent();

void _submitGenericGUIContext(GUIContext& guiContext, ResourceManager& manager,
                              Engine& engine, Window& window);
void _submitTextGUIContext(GUIContext& guiContext, ResourceManager& manager,
                           Engine& engine, Window& window);
void _submitButtonGUIContext(GUIContext& guiContext, ResourceManager& manager,
                             Engine& engine, Window& window);
void _submitPanelGUIContext(GUIContext& guiContext, ResourceManager& manager,
                            Engine& engine, Window& window);
void _submitSliderGUIContext(GUIContext& guiContext, ResourceManager& manager,
                             Engine& engine, Window& window);
void _submitSpriteGUIContext(GUIContext& guiContext, ResourceManager& manager,
                             Engine& engine, Window& window);
void _submitDebugRectGUIContext(GUIContext& guiContext,
                                ResourceManager& manager, Engine& engine,
                                Window& window);

GUIContext createGUIContext(ResourceManager& manager, Engine& engine,
                            Window& window, StitchedAtlas const* spriteAtlas);
// NOTE: can manually call render commands in desired order, this is just
// utility
void renderGUI(CommandContext& context, GUIContext& guiContext, Window& window);

void setupGUIContext(GUIContext& guiContext, ResourceManager& manager,
                     CommandContext& context, Engine& engine);
void updateGUIContext(GUIContext& guiContext, F32x2 windowDimensions);
void dropGUIContext(GUIContext& guiContext, Engine& engine);
}  // namespace Vivium
