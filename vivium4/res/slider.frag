#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec3 vForegroundColor;
layout(location = 1) in vec3 vSliderColor;
layout(location = 2) in float vContrast;
layout(location = 3) in float vPercent;
layout(location = 4) in vec2 vUVCoords;
layout(location = 5) in vec2 vSliderScale;
layout(location = 6) in float vSelectorScale;

// We'll use minimal rendering, can fancy it up with borders and etc. later
// Simple circle for the slider itself, slightly larger than the slider
//	Selector scale determines this relative to the slider dimensions
// Slider scale designates what percentage of the UV coordinates should actually be used
//	for the slider
// Contrast determines how much darker the slider base is compared to the foreground

// No actual background panel, we'd add that ourselves
// TODO: selector colour

void main() {
	// Bounds check if we're within the slider region
	//	assume slider region to be centered within our foreground panel
	// let sldierSCale.x = 0.8
	//	i.e., 0.1 on each side
	// so (1-x) * 0.5 gives lb, x + (1 - x) * 0.5 gives ub
	vec2 paddingScaleMin = (vec2(1.0) - vSliderScale) * 0.5;
	vec2 paddingScaleMax = vec(1.0) - paddingScaleMin;

	if (vUVCoords.x < paddingScaleMin.x || vUVCoords.x > paddingScaleMax.x ||
		vUVCoords.y < paddingScaleMin.y || vUVCoords.y > paddingScaleMax.y)
	{
		// Draw nothing
		color = vec4(0.0);
	} else {
		// Draw percentage bar (square for now)
		// TODO: rounded bar maths?
		float scaledPercentage = vPercent * vSliderScale.x + paddingScaleMin.x;
		if (scaledPercentage < vUVCoords.x) {
			color = vec4(vSliderColor, 1.0);
		} else {
			color = vec4(vForegroundColor, 1.0);
		}
	}
}