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
};

void main() {
	gl_Position = matrices.proj * matrices.view * vec4(inPosition + translation, 0.0, 1.0);
	vTextureCoordinates = inTextureCoordinates;
}