#include "box_filter.h"
#include <math.h>
#include <assert.h>

// 初始化方盒滤波
BoxFilter::BoxFilter(float radius) : radius(radius) {
  assert(radius > 0.0f);
}

// 得到相对像素中心的权重
float BoxFilter::getWeight(float x, float y) {
  if (fabs(x) <= radius && fabs(y) <= radius)
    return 1.0f;
  else
    return 0.0f;
}

// 保守整数支撑半径
int BoxFilter::getSupportRadius() {
  return (int)ceil(radius - 1e-5f);
}
