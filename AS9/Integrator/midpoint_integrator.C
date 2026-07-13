#include "midpoint_integrator.h"
#include "particle.h"
#include "forcefield.h"
#include <assert.h>

// Midpoint（改进 Euler）积分
// 半步：pm = pn + vn*(dt/2), vm = vn + a(pn,t)*(dt/2)
// 全步：pn+1 = pn + vm*dt, vn+1 = vn + a(pm, t+dt/2)*dt
void MidpointIntegrator::Update(Particle *particle, ForceField *forcefield, float t, float dt) {
  assert(particle != NULL);
  assert(forcefield != NULL);

  Vec3f pn = particle->getPosition();
  Vec3f vn = particle->getVelocity();
  float mass = particle->getMass();

  Vec3f a_n = forcefield->getAcceleration(pn, mass, t); // 当前加速度

  Vec3f pm = pn + (dt * 0.5f) * vn; // 中点位置
  Vec3f vm = vn + (dt * 0.5f) * a_n; // 中点速度

  Vec3f a_m = forcefield->getAcceleration(pm, mass, t + dt * 0.5f); // 中点加速度

  particle->setPosition(pn + dt * vm);
  particle->setVelocity(vn + dt * a_m);

  particle->increaseAge(dt);  // 更新粒子年龄
}
