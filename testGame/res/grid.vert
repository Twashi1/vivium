#version 460

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_tex_coords;

layout(set = 0, binding = 0) uniform transformation_matrices_t {
	mat4 view;
	mat4 proj;
} matrices;

layout(location = 0) out vec2 out_tex_coords;

struct per_sprite_data {
	vec2 translation;
	vec2 tex_coord_translation;
	float tex_coord_scale;
	float angle;
};

layout (std140, set = 0, binding = 1) readonly buffer sprite_data_t {
	per_sprite_data sprite_data[];
};

void main() {
	per_sprite_data data = sprite_data[gl_InstanceIndex];

	float cos_angle = cos(data.angle);
	float sin_angle = sin(data.angle);

	float nx = in_position.x * cos_angle - in_position.y * sin_angle;
	float ny = in_position.x * sin_angle + in_position.y * cos_angle;

	vec2 rotated_pos = vec2(nx, ny);
	vec2 translated_pos = rotated_pos + data.translation;

	vec2 translated_tex_coords = (data.tex_coord_scale * in_tex_coords) + data.tex_coord_translation;

	gl_Position = matrices.proj * matrices.view * vec4(
		translated_pos, 0.0, 1.0
	);

	out_tex_coords = translated_tex_coords;
}