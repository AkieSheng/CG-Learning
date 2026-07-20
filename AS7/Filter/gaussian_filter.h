#pragma once

#include "filter.h"

struct GaussianFilter final : Filter {
  GaussianFilter(float sigma);
  auto getWeight(float x, float y) -> float override;
  auto getSupportRadius() -> int override;

  float sigma{};
};
