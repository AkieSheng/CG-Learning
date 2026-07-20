#pragma once

#include "sampler.h"

struct UniformSampler final : Sampler {
  UniformSampler(int num_samples);
  auto getSamplePosition(int n) -> Vec2f override;

  int gridSize{};
};
