#version 460

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTextureCoords;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
};

struct SpriteData {
	vec2 position;
	vec2 scale;
	vec2 texturePosition;
	vec2 textureScale;
};

layout(std140, binding = 1) readonly buffer InstanceData {
	SpriteData instanceData[];
};

layout(location = 0) out vec2 vTextureCoords;

void main() {
	SpriteData spriteData = instanceData[gl_InstanceID];

	gl_Position = proj * view * vec4(
		inPosition * spriteData.scale + spriteData.position, 0.0, 1.0
	);

	vTextureCoords = inTextureCoords * spriteData.textureScale + spriteData.texturePosition;
}