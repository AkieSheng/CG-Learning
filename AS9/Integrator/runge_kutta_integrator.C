#include "runge_kutta_integrator.h"
#include "particle.h"
#include "forcefield.h"
#include <assert.h>

// 四阶 Runge-Kutta（对状态 (p, v)）
// dp/dt = v，dv/dt = a(p, t)
//
// k1: 在 (pn, t)，即 (pn, vn)
// k2: 在 (pn + k1p*dt/2, t+dt/2)，即 (pn + k1p*dt/2, vn + k1v*dt/2)
// k3: 在 (pn + k2p*dt/2, t+dt/2)，即 (pn + k2p*dt/2, vn + k2v*dt/2)
// k4: 在 (pn + k3p*dt,   t+dt)，即 (pn + k3p*dt, vn + k3v*dt)
//
// pn+1 = pn + dt/6 *(k1p + 2*k2p + 2*k3p + k4p)
// vn+1 = vn + dt/6 *(k1v + 2*k2v + 2*k3v + k4v)
void RungeKuttaIntegrator::Update(Particle *particle, ForceField *forcefield,
                                  float t, float dt) {
  assert(particle != NULL);
  assert(forcefield != NULL);

  Vec3f pn = particle->getPosition();
  Vec3f vn = particle->getVelocity();
  float mass = particle->getMass();
  float h = dt;
  float h2 = dt * 0.5f;

  // k1
  Vec3f k1p = vn;
  Vec3f k1v = forcefield->getAcceleration(pn, mass, t);

  // k2
  Vec3f p2 = pn + h2 * k1p;
  Vec3f v2 = vn + h2 * k1v;
  Vec3f k2p = v2;
  Vec3f k2v = forcefield->getAcceleration(p2, mass, t + h2);

  // k3
  Vec3f p3 = pn + h2 * k2p;
  Vec3f v3 = vn + h2 * k2v;
  Vec3f k3p = v3;
  Vec3f k3v = forcefield->getAcceleration(p3, mass, t + h2);

  // k4
  Vec3f p4 = pn + h * k3p;
  Vec3f v4 = vn + h * k3v;
  Vec3f k4p = v4;
  Vec3f k4v = forcefield->getAcceleration(p4, mass, t + h);

  // 加权合成
  float sixth = h / 6.0f;
  particle->setPosition(pn + sixth * (k1p + 2.0f * k2p + 2.0f * k3p + k4p));
  particle->setVelocity(vn + sixth * (k1v + 2.0f * k2v + 2.0f * k3v + k4v));

  particle->increaseAge(dt);
}
