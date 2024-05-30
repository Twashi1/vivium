#version 460

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUVPosition;
layout(location = 0) out vec2 vPosition;

layout(push_constant) uniform PushConstant {
	vec2 worldPosition;
	vec2 screenDimensions;
	float _1;
};

float pNorm(vec2 v, float power) {
	return sqrt(pow(abs(v.x), power) + pow(abs(v.y), power));
}

void main() {
	gl_Position = vec4(inPosition, 0.0, 1.0);

	// TODO: get actual screen ratio here
	vPosition = inUVPosition;
}