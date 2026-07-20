#pragma once

#include "object3d.h"
#include "vectors.h"


struct Triangle final : Object3D
{
  Triangle(Vec3f a, Vec3f b, Vec3f c, Material* m)
      : a(a), b(b), c(c) {
    material = m;
    Vec3f::Cross3(normal, b - a, c - a);
    normal.Normalize();
  }

  auto intersect(Ray const& r, Hit& h, float tmin) -> bool override;
  auto paint() const -> void override;

  Vec3f a{};
  Vec3f b{};
  Vec3f c{};
  Vec3f normal{};
};
