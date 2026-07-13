#include "hose_generator.h"
#include "particle.h"
#include <assert.h>

HoseGenerator::HoseGenerator(Vec3f position, float position_randomness,
                             Vec3f velocity, float velocity_randomness) {
  this->position = position;
  this->position_randomness = position_randomness;
  this->velocity = velocity;
  this->velocity_randomness = velocity_randomness;
}

Particle* HoseGenerator::Generate(float /*current_time*/, int /*i*/) {
  assert(rng != NULL);

  // 位置 / 速度在球状随机扰动下采样
  Vec3f p = position + position_randomness * rng->randomVector();
  Vec3f v = velocity + velocity_randomness * rng->randomVector();

  Vec3f c = jitterColor();  // 颜色扰动
  float m = jitterScalar(mass, mass_randomness);  // 质量扰动
  float life = jitterScalar(lifespan, lifespan_randomness);  // 寿命扰动

  return new Particle(p, v, c, dead_color, m, life);  // 生成粒子
}
