#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec2 vTexCoords;

layout(binding = 0) uniform Color {
	vec3 inputColor;
};

void main() {
	color = vec4(0.0, vTexCoords.xy, 1.0);
}