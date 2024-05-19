#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec2 vTextureCoords;

layout(binding = 0) uniform sampler2D framebuffer;

void main() {
	color = texture(framebuffer, vTextureCoords) + vec4(0.5, 0.0, 0.0, 1.0);
}