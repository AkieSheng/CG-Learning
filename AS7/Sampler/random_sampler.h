#ifndef _RANDOM_SAMPLER_H_
#define _RANDOM_SAMPLER_H_

#include "sampler.h"

// 像素内完全随机采样
class RandomSampler : public Sampler {

public:
  RandomSampler(int num_samples);
  virtual Vec2f getSamplePosition(int n);
};

#endif
