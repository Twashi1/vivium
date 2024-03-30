#version 450

layout(location = 0) out vec4 color;

layout(set = 0, binding = 1) uniform dynamic_data_t {
	vec3 u_color;
	vec2 position;
};

void main() {
	color = vec4(u_color.xyz, 1.0);
}