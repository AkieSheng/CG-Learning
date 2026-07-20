#pragma once

#include "filter.h"

struct BoxFilter final : Filter {
  BoxFilter(float radius);
  auto getWeight(float x, float y) -> float override;
  auto getSupportRadius() -> int override;

  float radius{};
};
