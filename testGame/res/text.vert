#version 450

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 vin_texture_coordinates;

layout(location = 0) out vec2 vout_texture_coordinates;

layout(binding = 0) uniform transformation_matrices_t {
	mat4 view;
	mat4 proj;
} matrices;

void main() {
	gl_Position = matrices.proj * matrices.view * vec4(in_position, 0.0, 1.0);
	vout_texture_coordinates = vin_texture_coordinates;
}