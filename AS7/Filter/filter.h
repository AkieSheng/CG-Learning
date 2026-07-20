#pragma once

#include "vectors.h"

struct Film;

struct Filter {
  Filter()
  { }
  virtual ~Filter()
  { }

  virtual auto getColor(int i, int j, Film* film) -> Vec3f;

  virtual auto getWeight(float x, float y) -> float = 0;

  virtual auto getSupportRadius() -> int = 0;
};
