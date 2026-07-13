#ifndef _RUNGE_KUTTA_INTEGRATOR_H_
#define _RUNGE_KUTTA_INTEGRATOR_H_

#include "integrator.h"

// 四阶 Runge-Kutta 积分
class RungeKuttaIntegrator : public Integrator {

public:
  RungeKuttaIntegrator() {}
  virtual ~RungeKuttaIntegrator() {}

  virtual void Update(Particle *particle, ForceField *forcefield, float t, float dt);
  virtual Vec3f getColor() const { return Vec3f(0, 0, 1); } // 蓝色
};

#endif
