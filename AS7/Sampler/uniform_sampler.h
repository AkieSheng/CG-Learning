#ifndef _UNIFORM_SAMPLER_H_
#define _UNIFORM_SAMPLER_H_

#include "sampler.h"

// 像素内规则网格采样
class UniformSampler : public Sampler {

public:
  UniformSampler(int num_samples);
  virtual Vec2f getSamplePosition(int n);

private:
  int gridSize;  // 网格边长 d，符合 d*d == numSamples
};

#endif
