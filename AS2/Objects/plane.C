#include "plane.h"
#include <cmath>

auto Plane::intersect(Ray const& r, Hit& h, float tmin) -> bool {
  auto denom = normal.Dot3(r.getDirection());
  if (::fabsf(denom) < 1e-6f) {
    return false;
  }

  auto t = (d - normal.Dot3(r.getOrigin())) / denom;
  if (t >= tmin && t < h.getT()) {
    h.set(t, material, normal, r);
    return true;
  }
  return false;
}
