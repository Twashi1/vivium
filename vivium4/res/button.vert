#version 450

layout(location = 0) in vec2 inPosition;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
};

struct ButtonData {
	vec2 position;
	vec2 scale;
	vec3 foregroundColor;
};

layout(std140, binding = 0) readonly buffer InstanceData {
	ButtonData[] buttonData;
};

void main() {
	ButtonData btn = buttonData[gl_InstanceID];

	gl_Position = proj * view * vec4(
		inPosition * btn.scale + btn.position, 0.0, 1.0
	);
}