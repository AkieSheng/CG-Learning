#pragma once

#include "force_field.h"

struct ConstantForceField final : ForceField {
  ConstantForceField(Vec3f force)
  { this->force = force; }
  ~ConstantForceField() override {}

  auto getAcceleration(Vec3f const& position, float mass, float t) const -> Vec3f override;

  Vec3f force{};
};
