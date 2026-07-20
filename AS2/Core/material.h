#pragma once

#include "vectors.h"

struct Material {
  Material(Vec3f const& d_color) { diffuseColor = d_color; }
  virtual ~Material() {}

  virtual auto getDiffuseColor() const -> Vec3f { return diffuseColor; }

  Vec3f diffuseColor{};
};
