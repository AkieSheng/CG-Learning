#include "jittered_sampler.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

// 初始化抖动采样器
JitteredSampler::JitteredSampler(int num_samples) : Sampler(num_samples) {
  gridSize = (int)(sqrtf((float)numSamples) + 0.5f);
  if (gridSize * gridSize != numSamples) {
    printf("[DEBUG] JitteredSampler: num_samples=%d is not a perfect square "
           "(nearest d=%d)\n", numSamples, gridSize);
    assert(0);
  }
}

// 获取第 n 个样本在像素内的 2D 偏移
Vec2f JitteredSampler::getSamplePosition(int n) {
  assert(n >= 0 && n < numSamples);
  // 在单元 (ix, iy) 内随机偏移：((ix + ξx)/d, (iy + ξy)/d)，ξ ∈ [0,1)
  int ix = n % gridSize;
  int iy = n / gridSize;
  float jx = rand() / (RAND_MAX + 1.0f);
  float jy = rand() / (RAND_MAX + 1.0f);
  float x = (ix + jx) / (float)gridSize;
  float y = (iy + jy) / (float)gridSize;
  return Vec2f(x, y);
}
