#include "runge_kutta_integrator.h"
#include <cassert>
#include "particle.h"
#include "forcefield.h"

auto RungeKuttaIntegrator::Update(Particle* particle, ForceField* forcefield, float t, float dt)
    -> void {
  assert(particle != nullptr);
  assert(forcefield != nullptr);

  auto pn = particle->getPosition();
  auto vn = particle->getVelocity();
  auto mass = particle->getMass();
  auto h = dt;
  auto h2 = dt * 0.5f;

  auto k1p = vn;
  auto k1v = forcefield->getAcceleration(pn, mass, t);

  auto p2 = pn + h2 * k1p;
  auto v2 = vn + h2 * k1v;
  auto k2p = v2;
  auto k2v = forcefield->getAcceleration(p2, mass, t + h2);

  auto p3 = pn + h2 * k2p;
  auto v3 = vn + h2 * k2v;
  auto k3p = v3;
  auto k3v = forcefield->getAcceleration(p3, mass, t + h2);

  auto p4 = pn + h * k3p;
  auto v4 = vn + h * k3v;
  auto k4p = v4;
  auto k4v = forcefield->getAcceleration(p4, mass, t + h);

  auto sixth = h / 6.0f;
  particle->setPosition(pn + sixth * (k1p + 2.0f * k2p + 2.0f * k3p + k4p));
  particle->setVelocity(vn + sixth * (k1v + 2.0f * k2v + 2.0f * k3v + k4v));
  particle->increaseAge(dt);
}
