#pragma once

#include "vectors.h"

struct Particle;
struct ForceField;

struct Integrator {
  Integrator()
  { }
  virtual ~Integrator()
  { }

  virtual auto Update(Particle* particle, ForceField* forcefield, float t, float dt) -> void = 0;
  virtual auto getColor() const -> Vec3f = 0;
};

#include "euler_integrator.h"
#include "midpoint_integrator.h"
#include "runge_kutta_integrator.h"
