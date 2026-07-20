#pragma once

#include "force_field.h"

struct WindForceField final : ForceField {
  WindForceField(float magnitude)
  { this->magnitude = magnitude; }
  ~WindForceField() override {}

  auto getAcceleration(Vec3f const& position, float mass, float t) const -> Vec3f override;

  float magnitude{};
};
