#include "random_sampler.h"
#include <stdlib.h>
#include <assert.h>

// 初始化随机采样器
RandomSampler::RandomSampler(int num_samples) : Sampler(num_samples) {}

// 在像素内均匀随机取点，用 RAND_MAX+1 使采样点落在 [0,1)
Vec2f RandomSampler::getSamplePosition(int n) {
  assert(n >= 0 && n < numSamples);
  float x = rand() / (RAND_MAX + 1.0f);
  float y = rand() / (RAND_MAX + 1.0f);
  return Vec2f(x, y);
}
