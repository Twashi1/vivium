#version 450

layout(location = 0) in vec2 in_position;

layout(set = 0, binding = 0) uniform transformation_matrices_t {
	mat4 view;
	mat4 proj;
} matrices;

layout(set = 0, binding = 1) uniform dynamic_data_t {
	vec3 color;
	vec2 position;
	float rotation;
};

void main() {
	float cos_angle = cos(rotation);
	float sin_angle = sin(rotation);

	float nx = in_position.x * cos_angle - in_position.y * sin_angle;
	float ny = in_position.x * sin_angle + in_position.y * cos_angle;

	vec2 rotated_pos = vec2(nx, ny);
	vec2 translated_pos = rotated_pos + position;

	gl_Position = matrices.proj * matrices.view * vec4(
		translated_pos, 0.0, 1.0
	);
}