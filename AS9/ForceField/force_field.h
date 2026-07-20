#pragma once

#include "vectors.h"

struct ForceField {
  ForceField() {}
  virtual ~ForceField() {}

  virtual auto getAcceleration(Vec3f const& position, float mass, float t) const -> Vec3f = 0;
};
