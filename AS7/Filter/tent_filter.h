#pragma once

#include "filter.h"

struct TentFilter final : Filter {
  TentFilter(float radius);
  auto getWeight(float x, float y) -> float override;
  auto getSupportRadius() -> int override;

  float radius{};
};
