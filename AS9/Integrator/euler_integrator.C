#include "euler_integrator.h"
#include <cassert>
#include "particle.h"
#include "forcefield.h"

auto EulerIntegrator::Update(Particle* particle, ForceField* forcefield, float t, float dt)
    -> void {
  assert(particle != nullptr);
  assert(forcefield != nullptr);

  auto p = particle->getPosition();
  auto v = particle->getVelocity();
  auto mass = particle->getMass();

  auto a = forcefield->getAcceleration(p, mass, t);

  particle->setPosition(p + dt * v);
  particle->setVelocity(v + dt * a);
  particle->increaseAge(dt);
}
