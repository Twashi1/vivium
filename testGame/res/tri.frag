#version 450

layout(location = 0) out vec4 color;

layout(set = 0, binding = 0) uniform ImageColor {
	vec3 color;
} imageColor;

void main() {
	color = vec4(imageColor.color.xyz, 1.0);
}