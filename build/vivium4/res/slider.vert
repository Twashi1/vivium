#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 inPosition;
layout(location = 0) out vec3 vForegroundColor;
layout(location = 1) out vec3 vSliderColor;
layout(location = 2) out float vContrast;
layout(location = 3) out float vPercent;
layout(location = 4) out vec2 vUVCoords;
layout(location = 5) out vec2 vSliderScale;
layout(location = 6) out float vSelectorScale;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
};

struct SliderData {
	vec2 position;		  // 8
	vec2 scale;			  // 16
	vec3 foregroundColor; // 28
	float percent;	      // 32
	vec3 sliderColor;	  // 44
	float contrast;	      // 48
	vec2 sliderScale;	  // 56
	float selectorScale;  // 60
	float _fill0;		  // 64
};

layout(std140, binding = 0) readonly buffer InstanceData {
	SliderData[] sliderData;
};

void main() {
	SliderData slider = sliderData[gl_InstanceIndex];

	gl_Position = proj * view * vec4(
		inPosition * slider.scale + slider.position, 0.0, 1.0
	);

	vForegroundColor = slider.foregroundColor;
	vSliderColor  = slider.sliderColor;
	vContrast = slider.contrast;
	vPercent = slider.percent;
	vUVCoords = inPosition;
	vSliderScale = slider.sliderScale;
	vSelectorScale = slider.selectorScale;
}