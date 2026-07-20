#pragma once

#include "vectors.h"

#include <iostream>

struct Ray final
{
  Ray() {}
  Ray(Vec3f const& orig, Vec3f const& dir)
  {
    origin = orig;
    direction = dir;
  }
  Ray(Ray const& r) { *this = r; }

  auto getOrigin() const -> Vec3f const& { return origin; }
  auto getDirection() const -> Vec3f const& { return direction; }
  auto pointAtParameter(float t) const -> Vec3f
  {
    return origin + direction * t;
  }

  Vec3f origin{};
  Vec3f direction{};
};

inline auto operator << (std::ostream& os, Ray const& r) -> std::ostream&
{
  os << "Ray <o:" << r.getOrigin() << ", d:" << r.getDirection() << ">";
  return os;
}
