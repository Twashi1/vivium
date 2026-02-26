#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec3 vColor;
layout(location = 1) in vec2 vPosition;

void main() {
	vec2 centered = vec2(0.5, 0.5) - vPosition;

	float magnitude = sqrt(centered.x * centered.x + centered.y * centered.y);
	float alpha;

	if (magnitude > 0.5) {
		alpha = 0.0;
	} else {
		alpha = 1.0;
	}

	color = vec4(vColor.xyz, alpha);
}
