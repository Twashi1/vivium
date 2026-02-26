#include "random.h"

#include <random>

namespace Vivium {
// Note, not random device since we don't care, not a cryptographic module
void _randomInit() { _randomGenerator = std::mt19937{0}; }

bool randomBool() {
  std::uniform_int_distribution<int> dist(0, 1);
  return dist(_randomGenerator);
}

int randomInt(int min, int max) {
  std::uniform_int_distribution<int> dist(min, max);
  return dist(_randomGenerator);
}

float randomFloat(float min, float max) {
  std::uniform_real_distribution<float> dist(min, max);
  return dist(_randomGenerator);
}

F32x2 randomVectorCircle(float radius) {
  std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

  // Expect this to take 1-2 iterations (1.27)
  for (int i = 0; i < 8; i++) {
    float x = dist(_randomGenerator);
    float y = dist(_randomGenerator);

    if (VIVIUM_LIKELY(x * x + y * y <= 1.0f)) {
      return F32x2(x * radius, y * radius);
    }
  }

  // Impossibly improbable path
  return F32x2(0.0f);
}

F32x2 randomVector(float min, float max) {
  std::uniform_real_distribution<float> dist(min, max);

  float x = dist(_randomGenerator);
  float y = dist(_randomGenerator);

  return F32x2(x, y);
}

F32x2 randomVectorCirucmference(float radius) {
  std::uniform_real_distribution<float> dist(0.0f, 2.0f * 3.14159265358f);

  float angle = dist(_randomGenerator);

  return F32x2(std::cos(angle) * radius, std::sin(angle) * radius);
}
}  // namespace Vivium
