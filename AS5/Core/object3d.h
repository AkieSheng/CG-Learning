#pragma once

#include "material.h"
#include "ray.h"
#include "hit.h"

struct BoundingBox;
struct Grid;
struct Matrix;

struct Object3D {
  Object3D() : material(nullptr), bbox(nullptr) {}
  virtual ~Object3D();

  virtual auto intersect(Ray const& r, Hit& h, float tmin) -> bool = 0;
  virtual auto intersectShadow(Ray const& r, float tmin, float tmax, float& t,
                               Material** outMaterial) -> bool = 0;
  virtual auto paint() const -> void = 0;

  auto getBoundingBox() const -> BoundingBox* { return bbox; }

  virtual auto insertIntoGrid(Grid* g, Matrix* m) -> void;
  virtual auto debugPrintBoundingBox(int depth) const -> void;

  Material* material{};
  BoundingBox* bbox{};
};
