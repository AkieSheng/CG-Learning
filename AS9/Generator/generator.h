#ifndef _GENERATOR_H_
#define _GENERATOR_H_

#include "vectors.h"
#include "random.h"

class Particle;

// 粒子生成器基类
class Generator {

public:

  Generator();
  virtual ~Generator();

  // 初始化公共粒子参数
  void SetColors(Vec3f color, Vec3f dead_color, float color_randomness);
  void SetLifespan(float lifespan, float lifespan_randomness, int desired_num_particles);
  void SetMass(float mass, float mass_randomness);

  // 新粒子数
  virtual int numNewParticles(float current_time, float dt) const;

  // 生成第 i 个新粒子
  virtual Particle* Generate(float current_time, int i) = 0;

  // 绘制
  virtual void Paint() const {}

  // 重置随机流
  virtual void Restart();

protected:

  // 随机扰动辅助
  Vec3f jitterColor();
  float jitterScalar(float base, float randomness);

  Vec3f color;  // 存在颜色
  Vec3f dead_color;  // 死亡颜色
  float color_randomness;  // 颜色随机扰动
  float mass;  // 质量
  float mass_randomness;  // 质量随机扰动
  float lifespan;  // 寿命
  float lifespan_randomness;  // 寿命随机扰动
  int desired_num_particles;  // 期望粒子数

  Random *rng;  // 随机数生成器

};

// 子类声明
#include "hose_generator.h"
#include "ring_generator.h"

#endif
