#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec2 vTextureCoordinates;

layout(binding = 0) uniform sampler2D textAtlasSampler;

layout(binding = 1) uniform Color {
	vec3 color;
} text;

void main() {
	vec4 sampled = texture(textAtlasSampler, vTextureCoordinates);

	color = vec4(text.color.xyz, 1.0) * sampled.r;
}