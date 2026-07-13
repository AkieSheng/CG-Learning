#ifndef _RING_GENERATOR_H_
#define _RING_GENERATOR_H_

#include "generator.h"

// 环形生成器
class RingGenerator : public Generator {

public:
  RingGenerator(float position_randomness,
                Vec3f velocity, float velocity_randomness);
  virtual ~RingGenerator() {}

  virtual int numNewParticles(float current_time, float dt) const;
  virtual Particle* Generate(float current_time, int i);
  virtual void Paint() const;

private:
  float position_randomness;  // 位置随机扰动
  Vec3f velocity;  // 速度
  float velocity_randomness;  // 速度随机扰动
};

#endif
