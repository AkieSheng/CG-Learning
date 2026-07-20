#include "midpoint_integrator.h"
#include <cassert>
#include "particle.h"
#include "forcefield.h"

auto MidpointIntegrator::Update(Particle* particle, ForceField* forcefield, float t, float dt)
    -> void {
  assert(particle != nullptr);
  assert(forcefield != nullptr);

  auto pn = particle->getPosition();
  auto vn = particle->getVelocity();
  auto mass = particle->getMass();

  auto a_n = forcefield->getAcceleration(pn, mass, t);

  auto pm = pn + (dt * 0.5f) * vn;
  auto vm = vn + (dt * 0.5f) * a_n;

  auto a_m = forcefield->getAcceleration(pm, mass, t + dt * 0.5f);

  particle->setPosition(pn + dt * vm);
  particle->setVelocity(vn + dt * a_m);
  particle->increaseAge(dt);
}
