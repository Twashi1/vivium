#include "slider.h"

namespace Vivium {
	Slider createSlider(GUIContext& guiContext, SliderSpecification specification)
	{
		Slider slider{};

		slider.base = createGUIElement(guiContext);
		slider.foregroundColor = specification.foregroundColor;
		slider.percent = specification.percent;
		slider.sliderColor = specification.sliderColor;
		slider.sliderScale = specification.sliderScale;
		slider.selectorScale = specification.selectorScale;

		addChild(specification.parent, { &slider.base, 1 }, guiContext);
		
		return slider;
	}

	void updateSlider(Slider& slider, GUIContext& guiContext)
	{
		F32x2 cursorPos = Input::getCursor();

		if (pointInElement(cursorPos, properties(slider, guiContext)) && Input::get(Input::BTN_1)) {
			// Calculate x percentage relative to true position and dimension
			float percent = (cursorPos.x - properties(slider, guiContext).truePosition.x) / properties(slider, guiContext).trueDimensions.x;
			slider.percent = std::clamp(percent, 0.0f, 1.0f);
		}
	}

	void submitSliders(std::span<Slider*> const sliders, GUIContext& guiContext)
	{
		for (uint64_t i = 0; i < sliders.size(); i++) {
			Slider& slider = *sliders[i];

			_GUISliderInstanceData instance;
			instance.position = properties(slider.base, guiContext).truePosition;
			instance.scale = properties(slider.base, guiContext).trueDimensions;

			instance.foregroundColor = slider.foregroundColor;
			instance.percent = slider.percent;
			instance.sliderColor = slider.sliderColor;
			instance.sliderScale = slider.sliderScale;
			instance.selectorScale = slider.selectorScale;

			guiContext.slider.sliders.push_back(instance);
		}
	}

	void renderSliders(CommandContext& context, GUIContext& guiContext, Window& window)
	{
		Perspective perspective = orthogonalPerspective2D(windowDimensions(window), F32x2(0.0f), 0.0f, 1.0f);

		setBuffer(guiContext.slider.storageBuffer.resource, 0, guiContext.slider.sliders.data(), guiContext.slider.sliders.size() * sizeof(_GUISliderInstanceData));
		cmdBindPipeline(context, guiContext.slider.pipeline.resource);
		cmdBindVertexBuffer(context, guiContext.rectVertexBuffer.resource);
		cmdBindIndexBuffer(context, guiContext.rectIndexBuffer.resource);
		cmdBindDescriptorSet(context, guiContext.slider.descriptorSet.resource, guiContext.slider.pipeline.resource);
		cmdWritePushConstants(context, &perspective, sizeof(Perspective), 0, ShaderStage::VERTEX, guiContext.slider.pipeline.resource);
		cmdDrawIndexed(context, 6, guiContext.slider.sliders.size());
		
		guiContext.slider.sliders.clear();
	}

	float getSliderValue(Slider& slider, float min, float max)
	{
		return slider.percent * (max - min) + min;
	}
}