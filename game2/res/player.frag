#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec2 vTexCoords;

layout(binding = 0) uniform PlayerUniform {
	vec3 playerColor;
	float scale;
	vec2 _;
	float time;
};

const float radius = 0.5;
const float bloomRadius = 0.2;
const float totalRadius = radius + bloomRadius;

float slimeFunction(vec2 position, float height) {
	return (position.x) * (position.x) + height * position.y * position.y;
}

void main() {
	float slimeValue = slimeFunction((vTexCoords + vec2(-0.5, 0)) / vec2(totalRadius), 0.2 * sin(2.0 * time) + 1.0);
	float alpha = max(0, 1 - max(0, slimeValue - radius * radius) / bloomRadius);

	color = vec4(playerColor, alpha);
}