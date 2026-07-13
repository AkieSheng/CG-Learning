#ifndef _EULER_INTEGRATOR_H_
#define _EULER_INTEGRATOR_H_

#include "integrator.h"

// Euler 积分
// p_{n+1} = p_n + v_n * dt
// v_{n+1} = v_n + a(p_n, t) * dt
class EulerIntegrator : public Integrator {

public:
  EulerIntegrator() {}
  virtual ~EulerIntegrator() {}

  virtual void Update(Particle *particle, ForceField *forcefield,
                      float t, float dt);
  virtual Vec3f getColor() const { return Vec3f(1, 0, 0); } // 红色
};

#endif
