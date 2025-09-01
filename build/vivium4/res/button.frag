#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec3 vColor;

void main() {
	color = vec4(vColor.xyz, 1.0);
}