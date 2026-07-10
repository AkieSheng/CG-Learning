#ifndef _SAMPLER_H_
#define _SAMPLER_H_

#include <assert.h>
#include "vectors.h"

// 像素内超采样策略抽象基类
class Sampler {

public:
  Sampler(int num_samples) : numSamples(num_samples) {
    assert(numSamples > 0);
  }
  virtual ~Sampler() {}

  // 获取每像素样本数
  int getNumSamples() const { return numSamples; }

  // 获取第 n 个样本在像素内的 2D 偏移
  virtual Vec2f getSamplePosition(int n) = 0;

protected:
  int numSamples;  // 每像素样本数
};

#endif
