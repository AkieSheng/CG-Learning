#include "jittered_sampler.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>

JitteredSampler::JitteredSampler(int num_samples) : Sampler(num_samples)
{
  gridSize = static_cast<int>(::sqrtf((float)numSamples) + 0.5f);
  if (gridSize * gridSize != numSamples)
  {
    ::printf("[DEBUG] JitteredSampler: num_samples=%d is not a perfect square "
           "(nearest d=%d)\n", numSamples, gridSize);
    assert(0);
  }
}

Vec2f JitteredSampler::getSamplePosition(int n)
{
  assert(n >= 0 && n < numSamples);

  int ix = n % gridSize;
  int iy = n / gridSize;
  float jx = ::rand() / (RAND_MAX + 1.0f);
  float jy = ::rand() / (RAND_MAX + 1.0f);
  float x = (ix + jx) / (float)gridSize;
  float y = (iy + jy) / (float)gridSize;
  return Vec2f(x, y);
}
