#pragma once

#include "material.h"
#include "ray.h"
#include "hit.h"


struct Object3D
{
  Object3D() : material(nullptr) {}
  virtual ~Object3D() {}

  virtual auto intersect(Ray const& r, Hit& h, float tmin) -> bool = 0;
  virtual auto paint() const -> void = 0;

  Material* material{};
};
