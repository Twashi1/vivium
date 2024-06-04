#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec2 vPosition;

layout(binding = 0) uniform GroundUniform {
    float time;
};

const vec3 groundColor = vec3(0.6, 0.2, 0.1);
const float PI = radians(180);
const float TAU = 2 * PI;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

// From https://www.shadertoy.com/view/MslGD8
vec2 rand2(vec2 p) {
    p = vec2(dot(p,vec2(127.1,311.7)),
             dot(p,vec2(269.5,183.3)));
    return fract(sin(p)*18.5453);
}

float pNorm(vec2 v, float power) {
    return sqrt(pow(abs(v.x), power) + pow(abs(v.y), power));
}

float worleyNoise(vec2 inPosition, float gridSize) {
    const float sharpness = 1.75;
    const float luminosityOffset = 0.2;
    const float wobbleFrequency = 1.0 / 24.0;
    const float wobbleAmplitude = 0.4f;

    vec2 samplePosition = inPosition / gridSize;
    vec2 latticeOrigin = floor(samplePosition);

    float minimumDistance = 1.0 / 0.0;

    for (int dy = -2; dy <= 2; dy++)
    for (int dx = -2; dx <= 2; dx++)
    {
        vec2 latticePoint = latticeOrigin + vec2(dx, dy);
        vec2 randomOffset = rand2(latticePoint);
        vec2 timeOffset = vec2(
            sin((time * wobbleFrequency + randomOffset.x) * TAU),
            sin((time * wobbleFrequency + randomOffset.y) * TAU)
        );
        
        vec2 point = latticePoint + randomOffset + timeOffset;
        vec2 distanceVector = point - samplePosition;
        float distanceSquared = dot(distanceVector, distanceVector);

        if (distanceSquared < minimumDistance * minimumDistance) {
            minimumDistance = sqrt(distanceSquared);
        }
    }

    float value = min(1, pow(minimumDistance, sharpness) + luminosityOffset);

    return value;
}

void main() {
    color = vec4(groundColor.rgb, 1.0);
}