#pragma once

#include "integrator.h"

struct MidpointIntegrator final : Integrator {
  MidpointIntegrator() {}
  ~MidpointIntegrator() override {}

  auto Update(Particle* particle, ForceField* forcefield, float t, float dt) -> void override;
  auto getColor() const -> Vec3f override { return Vec3f(0, 1, 0); }
};
