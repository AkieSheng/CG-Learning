#ifndef _JITTERED_SAMPLER_H_
#define _JITTERED_SAMPLER_H_

#include "sampler.h"

// 抖动采样
// 在均匀网格的每个单元内再随机偏移
class JitteredSampler : public Sampler {

public:
  JitteredSampler(int num_samples);
  virtual Vec2f getSamplePosition(int n);

private:
  int gridSize;  // 网格边长 d
};

#endif
