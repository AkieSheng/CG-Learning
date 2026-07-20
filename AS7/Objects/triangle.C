#include "triangle.h"
#include "grid.h"
#include "boundingbox.h"
#include "matrix.h"
#include "raytracing_stats.h"
#include <math.h>

Triangle::Triangle(Vec3f a, Vec3f b, Vec3f c, Material *m)
    : a(a), b(b), c(c) {
  material = m;
  Vec3f::Cross3(normal, b - a, c - a);
  normal.Normalize();

  Vec3f bbMin(::fminf(::fminf(a.x(), b.x()), c.x()),
              ::fminf(::fminf(a.y(), b.y()), c.y()),
              ::fminf(::fminf(a.z(), b.z()), c.z()));
  Vec3f bbMax(::fmaxf(::fmaxf(a.x(), b.x()), c.x()),
              ::fmaxf(::fmaxf(a.y(), b.y()), c.y()),
              ::fmaxf(::fmaxf(a.z(), b.z()), c.z()));
  bbox = new BoundingBox(bbMin, bbMax);
}

void Triangle::insertIntoGrid(Grid *g, Matrix *m) {
  if (g == nullptr)
    return;

  if (m == nullptr) {
    Object3D::insertIntoGrid(g, nullptr);
    return;
  }

  Vec3f v0 = a, v1 = b, v2 = c;
  m->Transform(v0);
  m->Transform(v1);
  m->Transform(v2);

  Vec3f wmin(::fminf(::fminf(v0.x(), v1.x()), v2.x()),
             ::fminf(::fminf(v0.y(), v1.y()), v2.y()),
             ::fminf(::fminf(v0.z(), v1.z()), v2.z()));
  Vec3f wmax(::fmaxf(::fmaxf(v0.x(), v1.x()), v2.x()),
             ::fmaxf(::fmaxf(v0.y(), v1.y()), v2.y()),
             ::fmaxf(::fmaxf(v0.z(), v1.z()), v2.z()));

  g->insertObjectInWorldAABB(wmin, wmax, this, m);
}

bool Triangle::intersect(Ray const& r, Hit& h, float tmin) {
  RayTracingStats::IncrementNumIntersections();

  Vec3f edge1 = b - a;
  Vec3f edge2 = c - a;
  Vec3f pvec;
  Vec3f::Cross3(pvec, r.getDirection(), edge2);
  float det = edge1.Dot3(pvec);
  if (::fabs(det) < 1e-8f)
    return false;

  float invDet = 1.0f / det;
  Vec3f tvec = r.getOrigin() - a;
  float u = tvec.Dot3(pvec) * invDet;
  if (u < 0.0f || u > 1.0f)
    return false;

  Vec3f qvec;
  Vec3f::Cross3(qvec, tvec, edge1);
  float v = r.getDirection().Dot3(qvec) * invDet;
  if (v < 0.0f || u + v > 1.0f)
    return false;

  float t = edge2.Dot3(qvec) * invDet;
  if (t >= tmin && t < h.getT()) {
    h.set(t, material, normal, r);
    return true;
  }
  return false;
}

bool Triangle::intersectShadow(Ray const&r, float tmin, float tmax, float &t,
                               Material **outMaterial) {

  Vec3f edge1 = b - a;
  Vec3f edge2 = c - a;
  Vec3f pvec;
  Vec3f::Cross3(pvec, r.getDirection(), edge2);
  float det = edge1.Dot3(pvec);
  if (::fabs(det) < 1e-8f)
    return false;

  float invDet = 1.0f / det;
  Vec3f tvec = r.getOrigin() - a;
  float u = tvec.Dot3(pvec) * invDet;
  if (u < 0.0f || u > 1.0f)
    return false;

  Vec3f qvec;
  Vec3f::Cross3(qvec, tvec, edge1);
  float v = r.getDirection().Dot3(qvec) * invDet;
  if (v < 0.0f || u + v > 1.0f)
    return false;

  float hitT = edge2.Dot3(qvec) * invDet;
  if (hitT >= tmin && hitT <= tmax) {
    t = hitT;
    if (outMaterial != nullptr)
      *outMaterial = material;
    return true;
  }
  return false;
}
