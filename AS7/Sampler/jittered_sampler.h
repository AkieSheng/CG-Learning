#pragma once

#include "sampler.h"

struct JitteredSampler final : Sampler {
  JitteredSampler(int num_samples);
  auto getSamplePosition(int n) -> Vec2f override;

  int gridSize{};
};
