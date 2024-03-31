#version 460

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoords;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
} matrices;

layout(location = 0) out vec2 vTexCoords;

void main() {
	gl_Position = matrices.proj * matrices.view * vec4(
		inPosition, 0.0, 1.0
	);

	vTexCoords = inTexCoords;
}