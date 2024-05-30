#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec2 vPosition;

layout(push_constant) uniform SkyData {
    vec2 worldPosition;
    vec2 screenDimensions;
    float time;
};

const vec3 starColor = vec3(0.9, 0.8, 0.4);
const vec3 skyColor = vec3(0.05, 0.0, 0.2);
const float starPower = 0.4;
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

vec3 calculateBackgroundColor(vec2 inPosition) {
    const float octaves = 4;

    vec3 sumColor = vec3(0.0);

    float currentAmplitude = 1.0;
    float currentGridSize = 256.0;

    float sumAmplitude = 0.0;

    for (int i = 0; i < octaves; i++) {
        sumColor += skyColor * currentAmplitude * worleyNoise(inPosition, currentGridSize);

        sumAmplitude += currentAmplitude;
        currentAmplitude *= 0.5;
        currentGridSize *= 0.5;
    }

    return sumColor * (1 / sumAmplitude);
}

// TODO: parallax on stars
vec3 calculateSkyColor(vec2 inPosition) {
    const float gridSize = 25.0;
    const float starRadius = 0.95;
    const float bloomRadius = 0.15;
    const float starChance = 0.025;
    const float starPower = 0.4;

    const float starRadiusAmplitude = 0.025;
    const float starRadiusFrequency = 1.0 / 4.0;

    const float luminosityAmplitude = 0.05;
    const float luminosityFrequency = 1.0 / 16.0;
    const float luminosityRandomAmplitude = 0.2;
    const float luminosityOffset = 0.4;

    vec2 samplePosition = inPosition / gridSize;
    vec2 latticeOrigin = floor(samplePosition);

    float maxStarlight = 0.0f;
    vec3 backgroundColor = calculateBackgroundColor(inPosition);
    vec3 currentSkyColor = backgroundColor;

    for (int dy = -2; dy <= 2; dy++)
    for (int dx = -2; dx <= 2; dx++)
    {
        vec2 latticePoint = latticeOrigin + vec2(dx, dy);

        // Check star spawned in this cell
        if (rand(latticePoint) > starChance) { continue; } 

        float currentStarRadius = starRadius + starRadiusAmplitude * sin(
            (time * starRadiusFrequency + rand(rand2(latticePoint))) * TAU
        );

        vec2 randomOffset = rand2(latticePoint);
        vec2 starLocation = latticePoint + randomOffset;

        vec2 vectorToStar = starLocation - samplePosition;
        float starValue = pNorm(vectorToStar, starPower);
        float luminosity = luminosityOffset + 
            luminosityRandomAmplitude * rand(latticePoint)+
            luminosityAmplitude * sin((time * luminosityFrequency + 0.9578292) * TAU);

        // TODO: smoothstep interpolation?
        float mixValue = max(0, 1 - max(0, starValue - currentStarRadius) / bloomRadius);

        if (mixValue > maxStarlight) {
            maxStarlight = mixValue;
            currentSkyColor = mix(backgroundColor, starColor * luminosity, mixValue);
        }
    }

    return currentSkyColor;
}

void main() {
    // Transform position
    vec2 samplePosition = vPosition * screenDimensions + worldPosition;
    // Pixelate
    const float pixelSize = 4.0f;
    samplePosition = floor(samplePosition / pixelSize) * pixelSize;

    color = vec4(calculateSkyColor(samplePosition).rgb, 1.0);
}