#include "tent_filter.h"
#include <math.h>
#include <assert.h>

// 初始化帐篷滤波
TentFilter::TentFilter(float radius) : radius(radius) {
  assert(radius > 0.0f);
}

// 得到相对像素中心的权重，w = max(0, 1 - d/r)，d = sqrt(x²+y²)
float TentFilter::getWeight(float x, float y) {
  float d = sqrtf(x * x + y * y);
  if (d >= radius)
    return 0.0f;
  return 1.0f - d / radius;
}

// 保守整数支撑半径
int TentFilter::getSupportRadius() {
  return (int)ceil(radius - 1e-5f);
}
