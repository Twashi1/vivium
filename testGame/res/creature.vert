#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTextureCoords;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
};

layout(location = 0) out vec2 vTextureCoords;

struct SpriteData {
	vec2 textureTranslation;
	vec2 textureScale;
	vec2 spriteTranslation;
	vec2 spriteScale;
	float spriteAngle;
	// Get us up to 48 bytes
	int _fill;
	int _fill2;
	int _fill3;
};

layout (std140, binding = 1) readonly buffer SpriteArray {
	SpriteData spriteData[];
};

void main() {
	SpriteData data = spriteData[gl_InstanceIndex];

	float cosAngle = cos(data.spriteAngle);
	float sinAngle = sin(data.spriteAngle);

	vec2 translatedPosition = vec2(
		cosAngle * inPosition.x - sinAngle * inPosition.y,
		sinAngle * inPosition.x + cosAngle * inPosition.y
	);

	gl_Position = proj * view * vec4(
		translatedPosition * data.spriteScale + data.spriteTranslation, 0.0, 1.0
	);

	vTextureCoords = inTextureCoords * data.textureScale + data.textureTranslation;
}