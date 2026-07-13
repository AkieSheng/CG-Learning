#ifndef _MIDPOINT_INTEGRATOR_H_
#define _MIDPOINT_INTEGRATOR_H_

#include "integrator.h"

// Midpoint 积分
// pm = pn + vn*(dt/2); vm = vn + a(pn,t)*(dt/2)
// pn+1 = pn + vm*dt; vn+1 = vn + a(pm, t+dt/2)*dt
class MidpointIntegrator : public Integrator {

public:
  MidpointIntegrator() {}
  virtual ~MidpointIntegrator() {}

  virtual void Update(Particle *particle, ForceField *forcefield, float t, float dt);
  virtual Vec3f getColor() const { return Vec3f(0, 1, 0); } // 绿色
};

#endif
