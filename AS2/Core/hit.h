#pragma once

#include <iostream>
#include "vectors.h"
#include "ray.h"

struct Material;

struct Hit final {
  Hit() { material = nullptr; }
  Hit(float _t, Material* m, Vec3f n) {
    t = _t;
    material = m;
    normal = n;
  }
  Hit(Hit const& h) {
    t = h.t;
    material = h.material;
    normal = h.normal;
    intersectionPoint = h.intersectionPoint;
  }
  ~Hit() {}

  auto getT() const -> float { return t; }
  auto getMaterial() const -> Material* { return material; }
  auto getNormal() const -> Vec3f { return normal; }
  auto getIntersectionPoint() const -> Vec3f { return intersectionPoint; }

  auto set(float _t, Material* m, Vec3f n, Ray const& ray) -> void {
    t = _t;
    material = m;
    normal = n;
    intersectionPoint = ray.pointAtParameter(t);
  }

  float t{};
  Material* material{};
  Vec3f normal{};
  Vec3f intersectionPoint{};
};

inline auto operator<<(std::ostream& os, Hit const& h) -> std::ostream& {
  os << "Hit <" << h.getT() << ", " << h.getNormal() << ">";
  return os;
}
