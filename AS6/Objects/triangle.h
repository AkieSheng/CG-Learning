#pragma once

#include "object3d.h"
#include "vectors.h"

struct Grid;
struct Matrix;

struct Triangle final : Object3D {
  Triangle(Vec3f a, Vec3f b, Vec3f c, Material* m);

  auto intersect(Ray const& r, Hit& h, float tmin) -> bool override;
  auto intersectShadow(Ray const& r, float tmin, float tmax, float& t,
                       Material** outMaterial) -> bool override;
  auto paint() const -> void override;
  auto insertIntoGrid(Grid* g, Matrix* m) -> void override;
  auto debugPrintBoundingBox(int depth) const -> void override;

  Vec3f a{};
  Vec3f b{};
  Vec3f c{};
  Vec3f normal{};
};
