#version 460

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoords;
layout(location = 0) out vec2 vTexCoords;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
} matrices;

layout(binding = 0) uniform CharacterData {
	vec2 translation;
	vec2 scale;
	float angle;
};

void main() {
	float sinAngle = sin(angle);
	float cosAngle = cos(angle);

	vec2 rotatedPosition = vec2(
		cosAngle * inPosition.x - sinAngle * inPosition.y,
		sinAngle * inPosition.x + cosAngle * inPosition.y
	);

	gl_Position = matrices.proj * matrices.view * vec4(
		rotatedPosition * scale + translation, 0.0, 1.0
	);

	vTexCoords = inTexCoords;
}