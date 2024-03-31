#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec2 vTextureCoords;

layout(binding = 0) uniform sampler2D spriteSheet;

void main() {
	color = texture(spriteSheet, vTextureCoords);
	// color = vec4(1.0, 1.0, 0.0, 1.0);
}