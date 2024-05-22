#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTextureCoordinates;

layout(location = 0) out vec2 vTextureCoordinates;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
} matrices;

layout(binding = 2) uniform TranslationData {
	vec2 translation;
	vec2 scale;
	vec2 scaleOrigin;
};

void main() {
	// Calculate vector from scaleOrigin to vertex
	vec2 originalVector = inPosition - scaleOrigin;
	// Scale it
	vec2 scaledVector = originalVector * scale;
	// Add back in origin and translation
	vec2 transformed = scaledVector + scaleOrigin + translation;

	gl_Position = matrices.proj * matrices.view * vec4(transformed, 0.0, 1.0);
	vTextureCoordinates = inTextureCoordinates;
}