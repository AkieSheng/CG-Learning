#include "triangle.h"
#include "grid.h"
#include "boundingbox.h"
#include "matrix.h"
#include "gl_headers.h"
#include <cmath>
#include <cstdio>

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

auto Triangle::debugPrintBoundingBox(int depth) const -> void {
  for (int i = 0; i < depth; i++)
    ::printf("  ");
  ::printf("Triangle: ");
  if (bbox == nullptr)
    ::printf("nullptr bounding box\n");
  else
    bbox->Print();
}

auto Triangle::computeWorldBounds(const Matrix *m, Vec3f &wmin,
                                  Vec3f &wmax) const -> void {
  Vec3f wa = a, wb = b, wc = c;
  if (m != nullptr) {
    m->Transform(wa);
    m->Transform(wb);
    m->Transform(wc);
  }

  wmin = Vec3f(::fminf(::fminf(wa.x(), wb.x()), wc.x()),
               ::fminf(::fminf(wa.y(), wb.y()), wc.y()),
               ::fminf(::fminf(wa.z(), wb.z()), wc.z()));
  wmax = Vec3f(::fmaxf(::fmaxf(wa.x(), wb.x()), wc.x()),
               ::fmaxf(::fmaxf(wa.y(), wb.y()), wc.y()),
               ::fmaxf(::fmaxf(wa.z(), wb.z()), wc.z()));
}

auto Triangle::insertIntoGrid(Grid *g, Matrix *m) -> void {
  if (g == nullptr)
    return;

  Vec3f wmin, wmax;
  computeWorldBounds(m, wmin, wmax);
  BoundingBox worldBox(wmin, wmax);
  g->insertObjectInBBox(&worldBox, this, nullptr);
}

auto Triangle::intersect(Ray const&r, Hit &h, float tmin) -> bool {
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

auto Triangle::intersectShadow(Ray const&r, float tmin, float tmax, float &t,
                               Material **outMaterial) -> bool {
  
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

auto Triangle::paint() const -> void {
  if (material != nullptr)
    material->glSetMaterial();

  glBegin(GL_TRIANGLES);
  glNormal3f(normal.x(), normal.y(), normal.z());
  glVertex3f(a.x(), a.y(), a.z());
  glVertex3f(b.x(), b.y(), b.z());
  glVertex3f(c.x(), c.y(), c.z());
  glEnd();
}
