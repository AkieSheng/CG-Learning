#pragma once

#include "object3d.h"
#include "vectors.h"


struct Sphere final : Object3D
{
  Sphere(Vec3f center, float radius, Material* m)
      : center(center), radius(radius)
  {
    material = m;
  }

  auto intersect(Ray const& r, Hit& h, float tmin) -> bool override;
  auto intersectShadow(Ray const& r, float tmin, float tmax, float& t,
                       Material** outMaterial) -> bool override;
  auto paint() const -> void override;

  Vec3f center{};
  float radius{};
};
