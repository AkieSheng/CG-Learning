#ifndef _HOSE_GENERATOR_H_
#define _HOSE_GENERATOR_H_

#include "generator.h"

// 软管生成器
class HoseGenerator : public Generator {

public:
  HoseGenerator(Vec3f position, float position_randomness,
                Vec3f velocity, float velocity_randomness);
  virtual ~HoseGenerator() {}

  virtual Particle* Generate(float current_time, int i);

private:
  Vec3f position;  // 位置
  float position_randomness;  // 位置随机扰动
  Vec3f velocity;  // 速度
  float velocity_randomness;  // 速度随机扰动
};

#endif
