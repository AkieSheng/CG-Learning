#ifndef _GAUSSIAN_FILTER_H_
#define _GAUSSIAN_FILTER_H_

#include "filter.h"

// 高斯滤波：w = exp(-d²/(2σ²))，d > 2σ 时钳制为 0
class GaussianFilter : public Filter {

public:
  GaussianFilter(float sigma);
  virtual float getWeight(float x, float y);
  virtual int getSupportRadius();

private:
  float sigma;  // 高斯核的标准差
};

#endif
