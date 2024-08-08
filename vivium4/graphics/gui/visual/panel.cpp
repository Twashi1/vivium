#include "panel.h"

namespace Vivium {
	Panel createPanel(GUI::Visual::Context::Handle guiContext, PanelSpecification specification)
	{
		Panel panel{};

		panel.base = GUI::Visual::Context::_allocateGUIElement(guiContext);
		panel.backgroundColor = specification.backgroundColor;
		panel.borderColor = specification.borderColor;
		panel.borderSize = specification.borderSize;
		
		if (specification.parent == nullptr) {
			specification.parent = guiContext->defaultParent;
		}

		addChild(specification.parent, panel.base);
		
		return panel;
	}

	void dropPanel(Panel& panel, GUI::Visual::Context::Handle guiContext)
	{
		GUI::Visual::Context::_dropGUIElement(panel.base, guiContext);
	}

	void renderPanels(const std::span<Panel*> panels, Commands::Context::Handle context, GUI::Visual::Context::Handle guiContext, Window::Handle window)
	{
		std::vector<GUI::Visual::Context::_PanelInstanceData> panelData(panels.size());

		for (uint64_t i = 0; i < panels.size(); i++) {
			Panel& panel = *panels[i];

			GUI::Visual::Context::_PanelInstanceData instance;
			instance.position = panel.base->properties.truePosition;
			instance.scale = panel.base->properties.trueDimensions;
			instance.backgroundColor = panel.backgroundColor;
			instance.borderColor = panel.borderColor;
			instance.borderSizePx = panel.borderSize;

			panelData[i] = instance;
		}

		Math::Perspective perspective = Math::orthogonalPerspective2D(window, F32x2(0.0f), 0.0f, 1.0f);

		setBuffer(guiContext->panel.storageBuffer.resource, 0, panelData.data(), panelData.size() * sizeof(GUI::Visual::Context::_PanelInstanceData));
		Commands::bindPipeline(context, guiContext->panel.pipeline.resource);
		Commands::bindVertexBuffer(context, guiContext->rectVertexBuffer.resource);
		Commands::bindIndexBuffer(context, guiContext->rectIndexBuffer.resource);
		Commands::bindDescriptorSet(context, guiContext->panel.descriptorSet.resource, guiContext->panel.pipeline.resource);
		Commands::pushConstants(context, &perspective, sizeof(Math::Perspective), 0, ShaderStage::VERTEX, guiContext->panel.pipeline.resource);
		Commands::drawIndexed(context, 6, panelData.size());
	}
}