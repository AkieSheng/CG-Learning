#include "sphere.h"

#include <cmath>

auto Sphere::intersect(Ray const& r, Hit& h, float tmin) -> bool
{
  auto oc = r.getOrigin() - center;
  auto dir = r.getDirection();
  auto a = dir.Dot3(dir);
  auto b = 2.0f * oc.Dot3(dir);
  auto c = oc.Dot3(oc) - radius * radius;
  auto discriminant = b * b - 4.0f * a * c;
  if (discriminant < 0.0f)
    return false;
  auto sqrt_disc = ::sqrtf(discriminant);
  auto hit = false;
  auto t = (-b - sqrt_disc) / (2.0f * a);
  if ((t >= tmin) && (t < h.getT())) {
    h.set(t, material, r);
    hit = true;
  }
  t = (-b + sqrt_disc) / (2.0f * a);
  if ((t >= tmin) && (t < h.getT())) {
    h.set(t, material, r);
    hit = true;
  }
  return hit;
}
