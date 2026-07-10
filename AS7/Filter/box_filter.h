#ifndef _BOX_FILTER_H_
#define _BOX_FILTER_H_

#include "filter.h"

// 盒式滤波：|x|<=r 且 |y|<=r 时权重为 1，否则为 0
class BoxFilter : public Filter {

public:
  BoxFilter(float radius);
  virtual float getWeight(float x, float y);
  virtual int getSupportRadius();

private:
  float radius;  // 像素中心到边界的正交距离
};

#endif
