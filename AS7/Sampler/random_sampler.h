#pragma once

#include "sampler.h"

struct RandomSampler final : Sampler {
  RandomSampler(int num_samples);
  auto getSamplePosition(int n) -> Vec2f override;
};
