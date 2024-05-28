#version 460

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUVPosition;
layout(location = 0) out vec2 vPosition;

void main() {
	gl_Position = vec4(inPosition, 0.0, 1.0);

	vPosition = inUVPosition;
}