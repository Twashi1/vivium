#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec3 vColor;
layout(location = 1) in vec2 vPosition;
layout(location = 2) in vec2 vVel;

void main() {
  const float radius = 0.5;
  const float lineLength = 0.4;
  const float lineThickness = 0.02;
  const vec3 lineColor = vec3(1.0);

  vec2 centered = vPosition - vec2(0.5, 0.5);

  float dist = length(centered);

  // circle with clean edges
  float circleAlpha = smoothstep(radius, radius - 0.01, dist);

  // direction line (assume normalised)
  vec2 dir = vVel;
  // Project fragment to line
  float t = dot(centered, dir);
  // clamp to line segment
  float clampedT = clamp(t, 0.0, lineLength);
  // find closest point on line
  vec2 closest = dir * clampedT;
  // distance from fragment to point/line
  float lineDist = length(centered - closest);
  // calculate mask for line
  float lineAlpha = smoothstep(lineThickness, 0.0, lineDist);

  // disable line
  lineAlpha = 0.0;

  vec3 circleColor = vColor;
  float outAlpha = max(circleAlpha, lineAlpha);

  vec3 outColor = (circleColor * circleAlpha * (1.0 - lineAlpha) + lineColor * lineAlpha) / max(outAlpha, 1e-5);

  color = vec4(outColor, outAlpha);
}
