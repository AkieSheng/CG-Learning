#pragma once

#include "force_field.h"

struct GravityForceField final : ForceField {
  GravityForceField(Vec3f gravity) { this->gravity = gravity; }
  ~GravityForceField() override {}

  auto getAcceleration(Vec3f const& position, float mass, float t) const -> Vec3f override;

private:
  Vec3f gravity{};
};
