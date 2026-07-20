#pragma once

#include "vectors.h"

struct Random final {
  Random(int seed = 0)
  { last = seed; }
  ~Random()
  { }

  auto next() -> float
  {
    last = (1366 * last + 150889) % 714025;
    return static_cast<float>(last) / static_cast<float>(714025);
  }

  auto randomVector() -> Vec3f
  {
    auto x = next() * 2.0f - 1.0f;
    auto y = next() * 2.0f - 1.0f;
    auto z = next() * 2.0f - 1.0f;
    return Vec3f(x, y, z);
  }

  int last{};
};
