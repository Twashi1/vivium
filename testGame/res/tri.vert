#version 460

layout(location = 0) in vec2 inPosition;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
} matrices;

void main() {
	gl_Position = matrices.proj * matrices.view * vec4(
		inPosition, 0.0, 1.0
	);
}