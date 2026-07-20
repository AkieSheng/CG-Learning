#pragma once

#include "integrator.h"

struct EulerIntegrator final : Integrator {
  EulerIntegrator()
  { }
  ~EulerIntegrator() override {}

  auto Update(Particle* particle, ForceField* forcefield, float t, float dt) -> void override;
  auto getColor() const -> Vec3f override { return Vec3f(1, 0, 0); }
};
