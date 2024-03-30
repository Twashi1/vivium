#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec2 out_tex_coords;

layout(binding = 2) uniform sampler2D tex_sampler;

void main() {
	color = texture(tex_sampler, out_tex_coords);
	// color = vec4(1.0, 1.0, 0.0, 1.0);
}