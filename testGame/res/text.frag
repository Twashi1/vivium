#version 450

layout(location = 0) in vec2 vout_texture_coordinates;

layout(location = 0) out vec4 color;
layout(binding = 1) uniform sampler2D tex_sampler;

layout(binding = 2) uniform text_color_t {
	vec3 text_color;
} t;

void main() {
	vec4 sampled = texture(tex_sampler,
		vout_texture_coordinates);

	color = vec4(t.text_color.xyz, 1.0) * sampled.r;
}