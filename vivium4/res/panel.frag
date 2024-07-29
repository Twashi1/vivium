#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec3 vColor;
layout(location = 1) in vec2 vUVCoords;
layout(location = 2) in vec3 vBorderColor;
layout(location = 3) in float vBorderSize;

void main() {
	color = vUVCoords.x < vBorderSize || (1 - vUVCoords.x) < vBorderSize ||
			vUVCoords.y < vBorderSize || (1 - vUVCoords.y) < vBorderSize ?
			vec4(vBorderColor.xyz, 1.0) : vec4(vColor.xyz, 1.0);
}