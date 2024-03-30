#version 460

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_tex_coords;

layout(push_constant) uniform matrices_t {
	mat4 view;
	mat4 proj;
};

layout(binding = 1) uniform sprite_data_t {
	vec2 translation;
	vec2 tex_coord_translation;
	vec2 tex_coord_scale;
	float scale;
	float rotation;
};

layout(location = 0) out vec2 out_tex_coords;

void main() {
	float cos_angle = cos(rotation);
	float sin_angle = sin(rotation);

	float nx = in_position.x * cos_angle - in_position.y * sin_angle;
	float ny = in_position.x * sin_angle + in_position.y * cos_angle;

	vec2 rotated_pos = vec2(nx, ny);
	vec2 translated_pos = rotated_pos + translation;

	vec2 translated_tex_coords = (tex_coord_scale * in_tex_coords) + tex_coord_translation;

	gl_Position = proj * view * vec4(
		translated_pos, 0.0, 1.0
	);

	out_tex_coords = translated_tex_coords;
}