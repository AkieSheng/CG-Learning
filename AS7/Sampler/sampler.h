#pragma once

#include <cassert>
#include "vectors.h"

struct Sampler {
  Sampler(int num_samples) : numSamples(num_samples)
  {
    assert(numSamples > 0);
  }
  virtual ~Sampler()
  { }

  auto getNumSamples() const -> int { return numSamples; }

  virtual auto getSamplePosition(int n) -> Vec2f = 0;

  int numSamples{};
};
