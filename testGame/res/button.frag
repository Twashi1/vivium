#version 450

layout(location = 0) out vec4 color;

layout(set = 0, binding = 0) uniform InstanceData {
	vec3 foregroundColor;
	vec2 position;
};

void main() {
	color = vec4(foregroundColor.xyz, 1.0);
}