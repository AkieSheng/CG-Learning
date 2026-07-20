#include "triangle.h"
#include "gl_headers.h"
#include <cmath>

auto Triangle::intersect(Ray const& r, Hit& h, float tmin) -> bool {
  auto edge1 = b - a;
  auto edge2 = c - a;
  Vec3f pvec;
  Vec3f::Cross3(pvec, r.getDirection(), edge2);
  auto det = edge1.Dot3(pvec);
  if (::fabs(det) < 1e-8f) {
    return false;
  }

  auto invDet = 1.0f / det;
  auto tvec = r.getOrigin() - a;
  auto u = tvec.Dot3(pvec) * invDet;
  if (u < 0.0f || u > 1.0f) {
    return false;
  }

  Vec3f qvec;
  Vec3f::Cross3(qvec, tvec, edge1);
  auto v = r.getDirection().Dot3(qvec) * invDet;
  if (v < 0.0f || u + v > 1.0f) {
    return false;
  }

  auto t = edge2.Dot3(qvec) * invDet;
  if (t >= tmin && t < h.getT()) {
    h.set(t, material, normal, r);
    return true;
  }
  return false;
}

auto Triangle::paint() const -> void {
  if (material != nullptr) {
    material->glSetMaterial();
  }

  ::glBegin(GL_TRIANGLES);
  ::glNormal3f(normal.x(), normal.y(), normal.z());
  ::glVertex3f(a.x(), a.y(), a.z());
  ::glVertex3f(b.x(), b.y(), b.z());
  ::glVertex3f(c.x(), c.y(), c.z());
  ::glEnd();
}
