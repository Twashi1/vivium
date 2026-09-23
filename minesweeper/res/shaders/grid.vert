#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTextureCoords;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
} matrices;

layout(location = 0) out vec2 vTextureCoords;

// TODO: texture scale and tile scale don't need to be defined per-tile
struct TileInstanceData {
	vec2 tileTranslation;
	vec2 textureTranslation;
	vec2 textureScale;
	vec2 tileScale;
};

layout (std140, set = 0, binding = 0) readonly buffer TileInstanceArray {
	TileInstanceData tileInstanceData[];
};

void main() {
	TileInstanceData data = tileInstanceData[gl_InstanceIndex];

	gl_Position = matrices.proj * matrices.view * vec4(
		inPosition * data.tileScale + data.tileTranslation, 0.0, 1.0
	);

	vTextureCoords = inTextureCoords * data.textureScale + data.textureTranslation;
}