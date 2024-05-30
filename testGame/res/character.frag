#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec2 vTexCoords;

layout(binding = 1) uniform sampler2D characterTexture;

void main() {
	color = texture(characterTexture, vTexCoords);
}