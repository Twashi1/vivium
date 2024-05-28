#version 450

layout(location = 0) out vec4 color;
layout(location = 0) in vec2 vPosition;

layout(push_constant) uniform SkyData {
    vec2 worldPosition;
};

const float gridSize = 20.0f;
const float starChance = 0.05f;
const float starRadius = 10.0f;
const vec3 backgroundColor = vec3(0.2, 0.0, 0.2);

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

// From https://www.shadertoy.com/view/MslGD8
vec2 hash(vec2 p) {
    p = vec2(dot(p,vec2(127.1,311.7)),
             dot(p,vec2(269.5,183.3)));
    return fract(sin(p)*18.5453);
}

float starFunction(vec2 v, float power) {
    return sqrt(pow(abs(v.x), power) + pow(abs(v.y * 9.0f/16.0f), power));
}

// TODO: parallax shift
void main() {
	vec2 latticeOrigin = floor(vPosition * gridSize);
	vec3 backgroundColor = vec3(0.05, 0.0, 0.2);
	vec4 skyColor = vec4(backgroundColor.xyz, 1.0);

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            vec2 latticePoint = latticeOrigin + vec2(dx, dy);
		if (rand(latticePoint) > starChance) { continue; }
            vec2 starOffset = hash(latticePoint);
            vec2 starLocation = latticePoint + starOffset;

            vec2 vectorToStar = starLocation - vPosition * gridSize;
            float starValue = starFunction(vectorToStar, 0.4);

            if (starValue < 1.0f) {
                skyColor = vec4(1.0, 1.0, 1.0, 1.0);
            }
        }
    }

	color = vec4(skyColor.xyzw);
}