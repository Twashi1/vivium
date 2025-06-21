#include "panel.h"

namespace Vivium {
	Panel createPanel(GUIContext& guiContext, PanelSpecification specification)
	{
		Panel panel{};

		panel.base = createGUIElement(guiContext);
		panel.backgroundColor = specification.backgroundColor;
		panel.borderColor = specification.borderColor;
		panel.borderSize = specification.borderSize;

		addChild(specification.parent, { &panel.base, 1 }, guiContext);
		
		return panel;
	}

	void renderPanels(const std::span<Panel*> panels, CommandContext& context, GUIContext& guiContext, Window& window)
	{
		std::vector<_GUIPanelInstanceData> panelData(panels.size());

		for (uint64_t i = 0; i < panels.size(); i++) {
			Panel& panel = *panels[i];

			_GUIPanelInstanceData instance;
			instance.position = properties(panel.base, guiContext).truePosition;
			instance.scale = properties(panel.base, guiContext).trueDimensions;

			instance.backgroundColor = panel.backgroundColor;
			instance.borderColor = panel.borderColor;
			instance.borderSizePx = panel.borderSize;

			panelData[i] = instance;
		}

		Perspective perspective = orthogonalPerspective2D(windowDimensions(window), F32x2(0.0f), 0.0f, 1.0f);

		setBuffer(guiContext.panel.storageBuffer.resource, 0, panelData.data(), panelData.size() * sizeof(_GUIPanelInstanceData));
		cmdBindPipeline(context, guiContext.panel.pipeline.resource);
		cmdBindVertexBuffer(context, guiContext.rectVertexBuffer.resource);
		cmdBindIndexBuffer(context, guiContext.rectIndexBuffer.resource);
		cmdBindDescriptorSet(context, guiContext.panel.descriptorSet.resource, guiContext.panel.pipeline.resource);
		cmdWritePushConstants(context, &perspective, sizeof(Perspective), 0, ShaderStage::VERTEX, guiContext.panel.pipeline.resource);
		cmdDrawIndexed(context, 6, panelData.size());
	}
}