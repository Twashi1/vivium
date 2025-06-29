#version 450

layout(location = 0) out vec4 color;
layout(location = 1) in vec2 vUVCoords;
layout(location = 2) in vec3 vBorderColor;
layout(location = 3) in vec2 vBorderSize;

// TODO: can massively clean up both shaders
void main() {
	color = vUVCoords.x < vBorderSize.x || (1 - vUVCoords.x) < vBorderSize.x ||
			vUVCoords.y < vBorderSize.y || (1 - vUVCoords.y) < vBorderSize.y ?
			vec4(vBorderColor.xyz, 1.0) : vec4(vBorderColor.xyz, 0.0);
}