#include "random_sampler.h"
#include <cassert>
#include <cstdlib>

RandomSampler::RandomSampler(int num_samples) : Sampler(num_samples)
{ }

Vec2f RandomSampler::getSamplePosition(int n)
{
  assert(n >= 0 && n < numSamples);
  float x = ::rand() / (RAND_MAX + 1.0f);
  float y = ::rand() / (RAND_MAX + 1.0f);
  return Vec2f(x, y);
}
