#version 450

layout(location = 0) in vec2 inPosition;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
};

layout(set = 0, binding = 0) uniform InstanceData {
	vec2 position;
	vec2 scale;
	vec3 foregroundColor;
};

void main() {
	gl_Position = proj * view * vec4(
		inPosition * scale + position, 0.0, 1.0
	);
}