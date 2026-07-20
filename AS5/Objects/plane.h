#pragma once

#include "object3d.h"
#include "vectors.h"

struct Grid;
struct Matrix;

struct Plane final : Object3D {
  Plane(Vec3f normal, float d, Material* m);

  auto intersect(Ray const& r, Hit& h, float tmin) -> bool override;
  auto intersectShadow(Ray const& r, float tmin, float tmax, float& t,
                       Material** outMaterial) -> bool override;
  auto paint() const -> void override;
  auto debugPrintBoundingBox(int depth) const -> void override;

  Vec3f normal{};
  float d{};
};
