#pragma once

#include "vectors.h"
#include "matrix.h"
#include "perlin_noise.h"

#include <cmath>
#include <cstdlib>

inline auto procOdd(int i) -> bool {
  return (i & 1) != 0;
}

inline auto envFloatOr(char const* name, float fallback) -> float {
  char const* e = ::getenv(name);
  if (e == nullptr || e[0] == '\0')
    return fallback;
  return static_cast<float>(::atof(e));
}

inline auto marbleFrequencyScale() -> float {
  return envFloatOr("MARBLE_FREQ_SCALE", 1.0f);
}

inline auto woodFrequencyScale() -> float {
  return envFloatOr("WOOD_FREQ_SCALE", 1.0f);
}

inline auto mapToTextureSpace(Matrix const* mapping, Vec3f const& world) -> Vec3f {
  if (mapping == nullptr)
    return world;
  Vec3f p = world;
  mapping->Transform(p);
  return p;
}

inline auto clamp01(float x) -> float {
  if (x < 0.0f)
    return 0.0f;
  if (x > 1.0f)
    return 1.0f;
  return x;
}

inline auto noiseToUnit(double n) -> float {
  return static_cast<float>(n) + 0.5f;
}

inline auto fractalNoise(Vec3f const& p, int octaves) -> double {
  if (octaves <= 0)
    return 0.0;
  double sum = 0.0;
  double weight = 1.0;
  double x = p.x(), y = p.y(), z = p.z();
  for (int i = 0; i < octaves; i++) {
    sum += weight * PerlinNoise::noise(x, y, z);
    x *= 2.0;
    y *= 2.0;
    z *= 2.0;
    weight *= 0.5;
  }
  return sum;
}

inline auto woodBlendWeight(Vec3f const& texPoint, int octaves,
                            float frequency, float amplitude,
                            float freqBoost = 1.0f) -> float {
  float radius = ::sqrtf(texPoint.y() * texPoint.y() + texPoint.z() * texPoint.z());
  double n = fractalNoise(texPoint, octaves);
  float freq = frequency * woodFrequencyScale() * freqBoost;
  float v = ::sinf(freq * radius + amplitude * static_cast<float>(n));
  return clamp01(v * 0.5f + 0.5f);
}

inline auto lerpVec3(Vec3f const& a, Vec3f const& b, float t) -> Vec3f {
  return a * (1.0f - t) + b * t;
}

inline auto lerpFloat(float a, float b, float t) -> float {
  return a * (1.0f - t) + b * t;
}
