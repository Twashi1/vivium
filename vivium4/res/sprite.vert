#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 inPosition;
layout(location = 0) out vec2 vTextureCoords;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
};

struct SpriteData {
	vec2 position;			// 8
	vec2 scale;				// 16
	vec2 texturePosition;	// 24
	vec2 textureScale;		// 32
};

layout(std140, binding = 0) readonly buffer InstanceData {
	SpriteData[] spriteData;
};

void main() {
	SpriteData sprite = spriteData[gl_InstanceIndex];

	gl_Position = proj * view * vec4(
		inPosition * sprite.scale + sprite.position, 0.0, 1.0
	);

	vTextureCoords = (sprite.textureScale * inPosition) + sprite.texturePosition;
}