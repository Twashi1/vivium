#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 inPosition;
layout(location = 0) out vec3 vColor;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
};

struct PanelData {
	vec2 position;
	vec2 scale;
	vec3 backgroundColor;
	float _fill0;
	float _fill1;
};

layout(std140, binding = 0) readonly buffer InstanceData {
	PanelData[] panelData;
};

void main() {
	PanelData panel = panelData[gl_InstanceIndex];

	gl_Position = proj * view * vec4(
		inPosition * panel.scale + panel.position, 0.0, 1.0
	);

	vColor = panel.backgroundColor;
}