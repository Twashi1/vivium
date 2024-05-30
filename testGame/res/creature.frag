#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec2 vTextureCoords;

layout(binding = 0) uniform sampler2D textureAtlas;

void main() {
	color = texture(textureAtlas, vTextureCoords);
}