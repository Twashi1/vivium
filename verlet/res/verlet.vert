#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 0) out vec3 vColor;
layout(location = 1) out vec2 vPosition;

layout(push_constant) uniform Matrices {
  mat4 view;
  mat4 proj;
};

struct PointData {
  vec2 position;
  vec2 scale;
  vec3 color;
  float _fill0;
};

layout(std140, binding = 0) readonly buffer InstanceData {
  PointData[] pointData;
};

void main() {
  PointData point = pointData[gl_InstanceIndex];

  gl_Position = proj * view * vec4(
        inPosition * point.scale + point.position, 0.0, 1.0
      );

  vColor = point.color;
  vPosition = inPosition;
}
