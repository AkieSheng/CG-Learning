#pragma once

#include "ray.h"
#include "vectors.h"

#include <iostream>

struct Material;

struct Hit final
{
  Hit() { material = nullptr; }
  Hit(float _t, Material* m)
  {
    t = _t;
    material = m;
  }
  Hit(Hit const& h)
  {
    t = h.t;
    material = h.material;
    intersectionPoint = h.intersectionPoint;
  }
  ~Hit() {}

  auto getT() const -> float { return t; }
  auto getMaterial() const -> Material* { return material; }
  auto getIntersectionPoint() const -> Vec3f { return intersectionPoint; }

  auto set(float _t, Material* m, Ray const& ray) -> void
  {
    t = _t;
    material = m;
    intersectionPoint = ray.pointAtParameter(t);
  }

  float t{};
  Material* material{};
  Vec3f intersectionPoint{};
};

inline auto operator << (std::ostream& os, Hit const& h) -> std::ostream&
{
  os << "Hit <t:" << h.getT() << ">";
  return os;
}
