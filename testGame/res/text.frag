#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec2 vTextureCoordinates;

layout(binding = 0) uniform sampler2D textAtlasSampler;

layout(binding = 1) uniform Color {
	vec3 color;
} text;

// TODO: uniforms
const float smoothingFactor = 1.0f / 16.0f;
const float thickness = 0.2f;
const float alphaThreshold = 0.5f - thickness;

void main() {
	vec4 sampled = texture(textAtlasSampler, vTextureCoordinates);

	// Alpha blended
	color = vec4(text.color.xyz, 1.0) * smoothstep(alphaThreshold - smoothingFactor, alphaThreshold + smoothingFactor, sampled.r);
	// Alpha tested
	// color = vec4(text.color.xyz, sampled.r > 0.5 ? 1.0 : 0.0);
}