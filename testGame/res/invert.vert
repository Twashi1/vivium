#version 460

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTextureCoords;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
};

layout(location = 0) out vec2 vTextureCoords;

void main() {
	gl_Position = proj * view * vec4(
		inPosition, 0.0, 1.0
	);

	vTextureCoords = inTextureCoords;
}