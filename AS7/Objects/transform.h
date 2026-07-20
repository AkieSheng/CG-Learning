#pragma once

#include "object3d.h"
#include "matrix.h"

struct Grid;

struct Transform final : Object3D {
  Transform(Matrix& m, Object3D* o);
  ~Transform() override;

  auto intersect(Ray const& r, Hit& h, float tmin) -> bool override;
  auto intersectShadow(Ray const& r, float tmin, float tmax, float& t,
                       Material** outMaterial) -> bool override;
  auto insertIntoGrid(Grid* g, Matrix* m) -> void override;

  Matrix matrix{};
  Matrix inverseMatrix{};
  Object3D* object{};
};
