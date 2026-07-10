#include "gaussian_filter.h"
#include <math.h>
#include <assert.h>

// 初始化高斯滤波
GaussianFilter::GaussianFilter(float sigma) : sigma(sigma) {
  assert(sigma > 0.0f);
}

// 得到相对像素中心的权重，w = exp(-d²/(2σ²))，d > 2σ 时降为 0
float GaussianFilter::getWeight(float x, float y) {
  float d2 = x * x + y * y;
  float twoSigma = 2.0f * sigma;
  if (d2 > twoSigma * twoSigma)
    return 0.0f;
  return expf(-d2 / (2.0f * sigma * sigma));
}

// 保守整数支撑半径
int GaussianFilter::getSupportRadius() {
  return (int)ceil(2.0f * sigma - 1e-5f);
}
