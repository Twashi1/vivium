#version 460

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoords;
layout(location = 0) out vec2 vTexCoords;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
};

layout(binding = 0) uniform PlayerUniform {
	vec3 color;
	float scale;
	vec2 position;
	float time;
};

void main() {
	gl_Position = proj * view * vec4(
		inPosition * scale + position, 0.0, 1.0
	);

	vTexCoords = inTexCoords;
}