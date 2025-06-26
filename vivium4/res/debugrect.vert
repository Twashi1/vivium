#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) out vec2 vUVCoords;
layout(location = 2) out vec3 vBorderColor;
layout(location = 3) out vec2 vBorderSize;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
};

struct DebugRectData {
	vec2 position;		// 8
	vec2 scale;			// 16
	vec3 borderColor;	// 28
	float borderSize;	// 32
};

layout(std140, binding = 0) readonly buffer InstanceData {
	DebugRectData[] debugRectData;
};

void main() {
	DebugRectData debugRect = debugRectData[gl_InstanceIndex];

	gl_Position = proj * view * vec4(
		inPosition * debugRect.scale + debugRect.position, 0.0, 1.0
	);

	vUVCoords = inPosition;
	vBorderColor = debugRect.borderColor;
	vBorderSize = vec2(debugRect.borderSize, debugRect.borderSize * debugRect.scale.x / debugRect.scale.y);
}