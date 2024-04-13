#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec2 vTextureCoordinates;

layout(binding = 0) uniform sampler2D textAtlasSampler;

layout(binding = 1) uniform Color {
	vec3 color;
} text;

const float smoothingFactor = 1.0f / 16.0f;

void main() {
	vec4 sampled = texture(textAtlasSampler, vTextureCoordinates);

	// Alpha blended
	color = vec4(text.color.xyz, 1.0) * smoothstep(0.5 - smoothingFactor, 0.5 + smoothingFactor, sampled.r);
	// Alpha tested
	// color = vec4(text.color.xyz, sampled.r > 0.5 ? 1.0 : 0.0);
}