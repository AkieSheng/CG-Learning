#pragma once

#include "object3d.h"
#include "vectors.h"

struct Plane final : Object3D
{
  Plane(Vec3f normal, float d, Material* m)
      : normal(normal), d(d)
  {
    material = m;
    normal.Normalize();
  }

  auto intersect(Ray const& r, Hit& h, float tmin) -> bool override;

  Vec3f normal{};
  float d{};
};
