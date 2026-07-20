#pragma once

#include "hit.h"
#include "material.h"
#include "ray.h"

struct Object3D
{
  Object3D() : material(nullptr) {}
  virtual ~Object3D() {}

  virtual auto intersect(Ray const& r, Hit& h, float tmin) -> bool = 0;

  Material* material{};
};
