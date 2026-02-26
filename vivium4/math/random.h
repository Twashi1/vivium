#pragma once

#include <random>

#include "../core.h"
#include "vec2.h"

namespace Vivium {

inline std::mt19937 _randomGenerator;

void _randomInit();

bool randomBool();
int randomInt(int min, int max);
float randomFloat(float min = 0.0f, float max = 1.0f);
F32x2 randomVectorCircle(float radius);
F32x2 randomVector(float min, float max);
F32x2 randomVectorCirucmference(float radius);
}  // namespace Vivium
