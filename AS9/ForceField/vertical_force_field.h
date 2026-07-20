#pragma once

#include "force_field.h"

struct VerticalForceField final : ForceField {
  VerticalForceField(float magnitude)
  { this->magnitude = magnitude; }
  ~VerticalForceField() override {}

  auto getAcceleration(Vec3f const& position, float mass, float t) const -> Vec3f override;

  float magnitude{};
};
