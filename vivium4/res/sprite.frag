#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec2 vTextureCoords;

layout(binding = 1) uniform sampler2D textureSampler;

void main() {
	color = texture(textureSampler, vTextureCoords);
}