#include "plane.h"
#include "gl_headers.h"
#include <cmath>

auto Plane::intersect(Ray const& r, Hit& h, float tmin) -> bool {
  auto denom = normal.Dot3(r.getDirection());
  if (::fabs(denom) < 1e-6f) {
    return false;
  }

  auto t = (d - normal.Dot3(r.getOrigin())) / denom;
  if (t >= tmin && t < h.getT()) {
    h.set(t, material, normal, r);
    return true;
  }
  return false;
}

auto Plane::intersectShadow(Ray const& r, float tmin, float tmax, float& t,
                            Material** outMaterial) -> bool {
  auto denom = normal.Dot3(r.getDirection());
  if (::fabs(denom) < 1e-6f) {
    return false;
  }

  auto hitT = (d - normal.Dot3(r.getOrigin())) / denom;
  if (hitT >= tmin && hitT <= tmax) {
    t = hitT;
    return true;
  }
  if (outMaterial != nullptr) {
    *outMaterial = material;
  }
  return false;
}

auto Plane::paint() const -> void {
  if (material != nullptr) {
    material->glSetMaterial();
  }

  auto v = (::fabs(normal.x()) < 0.9f) ? Vec3f(1, 0, 0) : Vec3f(0, 1, 0);
  Vec3f b1, b2;
  Vec3f::Cross3(b1, v, normal);
  b1.Normalize();
  Vec3f::Cross3(b2, normal, b1);
  b2.Normalize();

  auto origin = normal * d;
  constexpr auto big = 1000.0f;

  auto p0 = origin + b1 * (-big) + b2 * (-big);
  auto p1 = origin + b1 * big + b2 * (-big);
  auto p2 = origin + b1 * big + b2 * big;
  auto p3 = origin + b1 * (-big) + b2 * big;

  ::glBegin(GL_QUADS);
  ::glNormal3f(normal.x(), normal.y(), normal.z());
  ::glVertex3f(p0.x(), p0.y(), p0.z());
  ::glVertex3f(p1.x(), p1.y(), p1.z());
  ::glVertex3f(p2.x(), p2.y(), p2.z());
  ::glVertex3f(p3.x(), p3.y(), p3.z());
  ::glEnd();
}
