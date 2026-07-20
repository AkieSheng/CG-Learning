#include "uniform_sampler.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>

UniformSampler::UniformSampler(int num_samples) : Sampler(num_samples) {
  gridSize = static_cast<int>(::sqrtf((float)numSamples) + 0.5f);
  if (gridSize * gridSize != numSamples) {
    ::printf("[DEBUG] UniformSampler: num_samples=%d is not a perfect square "
           "(nearest d=%d)\n", numSamples, gridSize);
    assert(0);
  }
}

Vec2f UniformSampler::getSamplePosition(int n) {
  assert(n >= 0 && n < numSamples);

  int ix = n % gridSize;
  int iy = n / gridSize;

  float x = (ix + 0.5f) / (float)gridSize;
  float y = (iy + 0.5f) / (float)gridSize;
  return Vec2f(x, y);
}
