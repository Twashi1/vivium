#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTextureCoords;

layout(push_constant) uniform Matrices {
	mat4 view;
	mat4 proj;
} matrices;

layout(location = 0) out vec2 vTextureCoords;

layout(set = 0, binding = 1) uniform UniformData {
	vec2 textureScale;
	float tileScale;
};

// No alignment requirements https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap15.html#interfaces-resources-layout
struct TileInstanceData {
	vec2 tileTranslation;
	vec2 textureTranslation;
};

layout (std140, set = 0, binding = 2) readonly buffer TileInstanceArray {
	TileInstanceData tileInstanceData[];
};

void main() {
	TileInstanceData data = tileInstanceData[gl_InstanceIndex];

	gl_Position = matrices.proj * matrices.view * vec4(
		inPosition * tileScale + data.tileTranslation, 0.0, 1.0
	);

	vTextureCoords = inTextureCoords * textureScale + data.textureTranslation;
}