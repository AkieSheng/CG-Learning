#pragma once

#include "force_field.h"

struct RadialForceField final : ForceField {
  RadialForceField(float magnitude)
  { this->magnitude = magnitude; }
  ~RadialForceField() override {}

  auto getAcceleration(Vec3f const& position, float mass, float t) const -> Vec3f override;

  float magnitude{};
};
