#pragma once

#include "object3d.h"

struct Group final : Object3D {
  Group(int numObjects);
  ~Group();

  auto addObject(int index, Object3D* obj) -> void;
  auto intersect(Ray const& r, Hit& h, float tmin) -> bool override;

  Object3D** objects{};
  int numObjects{};
};
