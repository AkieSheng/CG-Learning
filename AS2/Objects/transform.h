#pragma once

#include "object3d.h"
#include "matrix.h"

struct Transform final : Object3D {
  Transform(Matrix& m, Object3D* o)
      : matrix(m), object(o) {
    matrix.Inverse(inverseMatrix);
    inverseMatrix.Transpose();
  }
  ~Transform() { delete object; }

  auto intersect(Ray const& r, Hit& h, float tmin) -> bool override;

  Matrix matrix{};
  Matrix inverseMatrix{};
  Object3D* object{};
};
