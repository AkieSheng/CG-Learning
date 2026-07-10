#ifndef _TENT_FILTER_H_
#define _TENT_FILTER_H_

#include "filter.h"

// 帐篷滤波：中心权重 1，欧氏距离超过 radius 时为 0，中间线性衰减
class TentFilter : public Filter {

public:
  TentFilter(float radius);
  virtual float getWeight(float x, float y);
  virtual int getSupportRadius();

private:
  float radius;  // 像素中心到边界的正交距离
};

#endif
