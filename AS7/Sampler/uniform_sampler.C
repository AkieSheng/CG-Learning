#include "uniform_sampler.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>

// 初始化均匀采样器
UniformSampler::UniformSampler(int num_samples) : Sampler(num_samples) {
  gridSize = (int)(sqrtf((float)numSamples) + 0.5f);  // 计算网格边长 d，d*d == numSamples
  if (gridSize * gridSize != numSamples) {
    printf("[DEBUG] UniformSampler: num_samples=%d is not a perfect square "
           "(nearest d=%d)\n", numSamples, gridSize);
    assert(0);
  }
}

// 获取第 n 个样本在像素内的 2D 偏移
Vec2f UniformSampler::getSamplePosition(int n) {
  assert(n >= 0 && n < numSamples);
  // 计算索引
  int ix = n % gridSize;
  int iy = n / gridSize;
  // 计算偏移
  float x = (ix + 0.5f) / (float)gridSize;
  float y = (iy + 0.5f) / (float)gridSize;
  return Vec2f(x, y);
}
