#pragma once

#include "object3d.h"
#include "vectors.h"

struct Grid;
struct Matrix;

struct Sphere final : Object3D {
  Sphere(Vec3f center, float radius, Material* m);

  auto intersect(Ray const& r, Hit& h, float tmin) -> bool override;
  auto intersectShadow(Ray const& r, float tmin, float tmax, float& t,
                       Material** outMaterial) -> bool override;
  auto insertIntoGrid(Grid* g, Matrix* m) -> void override;

  Vec3f center{};
  float radius{};
};
