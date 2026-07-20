#pragma once

#include "object3d.h"

struct Grid;
struct Matrix;

struct Group final : Object3D {
  Group(int numObjects);
  ~Group() override;

  auto addObject(int index, Object3D* obj) -> void;
  auto intersect(Ray const& r, Hit& h, float tmin) -> bool override;
  auto intersectShadow(Ray const& r, float tmin, float tmax, float& t,
                       Material** outMaterial) -> bool override;
  auto insertIntoGrid(Grid* g, Matrix* m) -> void override;

  Object3D** objects{};
  int numObjects{};
};
