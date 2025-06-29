#pragma once

#include "context.h"
#include "../../color.h"
#include "../../../input.h"

namespace Vivium {
	struct Slider {
		GUIElementReference base;
		Color foregroundColor;
		float percent;
		Color sliderColor;
		F32x2 sliderScale;
		float selectorScale;
	};

	struct SliderSpecification {
		GUIElementReference parent;
		Color foregroundColor;
		float percent;
		Color sliderColor;
		F32x2 sliderScale;
		float selectorScale;
	};

	Slider createSlider(GUIContext& guiContext, SliderSpecification specification);
	// NOTE: assumes slider already updated
	void updateSlider(Slider& slider, GUIContext& guiContext);
	
	void submitSliders(std::span<Slider*> const sliders, GUIContext& guiContext);
	void renderSliders(CommandContext& context, GUIContext& guiContext, Window& window);

	float getSliderValue(Slider& slider, float min, float max);
}