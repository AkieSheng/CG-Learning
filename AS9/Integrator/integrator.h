#ifndef _INTEGRATOR_H_
#define _INTEGRATOR_H_

#include "vectors.h"

class Particle;
class ForceField;

// 积分器基类
// 用步长 dt 推进粒子位置与速度，并更新年龄
class Integrator {

public:
  Integrator() {}
  virtual ~Integrator() {}

  // 推进粒子
  virtual void Update(Particle *particle, ForceField *forcefield,
                      float t, float dt) = 0;

  // -integrator_color 可视化时使用的积分器颜色
  virtual Vec3f getColor() const = 0;
};

// 子类声明
#include "euler_integrator.h"
#include "midpoint_integrator.h"
#include "runge_kutta_integrator.h"

#endif
