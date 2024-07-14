#include "panel.h"

namespace Vivium {
	Panel createPanel(GUI::Visual::Context::Handle guiContext, PanelSpecification const& specification)
	{
		Panel panel{};

		panel.base = GUI::Visual::Context::_allocateGUIElement(guiContext);
		panel.backgroundColor = specification.backgroundColor;

		return panel;
	}

	void renderPanels(const std::span<Panel*> panels, Commands::Context::Handle context, GUI::Visual::Context::Handle guiContext, Window::Handle window)
	{
		std::vector<GUI::Visual::Context::_PanelInstanceData> panelData(panels.size());

		for (uint64_t i = 0; i < panels.size(); i++) {
			Panel& panel = *panels[i];

			// TODO: modifies panels
			// TODO: shouldn't do this, should have some root element that gets updated by user (or automatically)
			_updateGUIElement(panel.base, nullptr, Window::dimensions(window));

			GUI::Visual::Context::_PanelInstanceData instance;
			instance.position = panel.base->properties.truePosition;
			instance.scale = panel.base->properties.trueDimensions;
			instance.backgroundColor = panel.backgroundColor;

			panelData[i] = instance;
		}

		Math::Perspective perspective = Math::orthogonalPerspective2D(window, F32x2(0.0f), 0.0f, 1.0f);

		setBuffer(guiContext->panel.storageBuffer.resource, 0, panelData.data(), panelData.size() * sizeof(GUI::Visual::Context::_PanelInstanceData));
		Commands::bindPipeline(context, guiContext->panel.pipeline.resource);
		Commands::bindVertexBuffer(context, guiContext->rectVertexBuffer.resource);
		Commands::bindIndexBuffer(context, guiContext->rectIndexBuffer.resource);
		Commands::bindDescriptorSet(context, guiContext->panel.descriptorSet.resource, guiContext->panel.pipeline.resource);
		Commands::pushConstants(context, &perspective, sizeof(Math::Perspective), 0, ShaderStage::VERTEX, guiContext->panel.pipeline.resource);
		Commands::drawIndexed(context, 6, panels.size());
	}
}