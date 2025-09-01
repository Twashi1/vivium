#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTextureCoordinates;
layout(location = 2) in vec3 inColor;

layout(location = 0) out vec2 vTextureCoordinates;
layout(location = 1) out vec3 vColor;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
} matrices;

void main() {
	gl_Position = matrices.proj * matrices.view * vec4(inPosition, 0.0, 1.0);
	
	vTextureCoordinates = inTextureCoordinates;
	vColor = inColor;
}