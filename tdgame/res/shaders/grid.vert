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
	vec2 tileTranslation; // 8
	vec2 tileScale; // 16
	vec2 textureTranslation; // 24
	vec2 textureScale; // 32
  float rotation;
  float _pad0;
  float _pad1;
  float _pad2; // 48 bytes total?
};

layout (std140, set = 0, binding = 0) readonly buffer TileInstanceArray {
	TileInstanceData tileInstanceData[];
};

void main() {
	TileInstanceData data = tileInstanceData[gl_InstanceIndex];

  float c = cos(data.rotation);
  float s = sin(data.rotation);

  mat2 rotationMatrix = mat2(c, -s, s, c);
  
  
  vec2 localPosition = inPosition - vec2(0.5);

  // Scale the tile.
  localPosition *= data.tileScale;

  // Rotate around the centre.
  localPosition = rotationMatrix * localPosition;

  // Move the centre to its world position.
  localPosition += data.tileTranslation + vec2(0.5) * data.tileScale;

	gl_Position = matrices.proj * matrices.view * vec4(
		 localPosition, 0.0, 1.0
	);

  vec2 uv = inTextureCoords * data.textureScale
          + data.textureTranslation;

  uv.y = data.textureTranslation.y +
         data.textureScale.y * (1.0 - inTextureCoords.y);

  vTextureCoords = uv;
}
