#include "plane.h"
#include "grid.h"
#include "raytracing_stats.h"
#include <math.h>

Plane::Plane(Vec3f normal, float d, Material *m)
    : normal(normal), d(d) {
  material = m;
  normal.Normalize();
  bbox = nullptr;
}

void Plane::insertIntoGrid(Grid *g, Matrix *m) {
  if (g != nullptr)
    g->addInfiniteObject(this);
}

bool Plane::intersect(Ray const&r, Hit &h, float tmin) {
  RayTracingStats::IncrementNumIntersections();
  float denom = normal.Dot3(r.getDirection());
  if (::fabs(denom) < 1e-6f)
    return false;

  float t = (d - normal.Dot3(r.getOrigin())) / denom;
  if (t >= tmin && t < h.getT()) {
    h.set(t, material, normal, r);
    return true;
  }
  return false;
}

bool Plane::intersectShadow(Ray const&r, float tmin, float tmax, float &t,
                            Material **outMaterial) {
  float denom = normal.Dot3(r.getDirection());
  if (::fabs(denom) < 1e-6f)
    return false;

  float hitT = (d - normal.Dot3(r.getOrigin())) / denom;
  if (hitT >= tmin && hitT <= tmax) {
    t = hitT;
    if (outMaterial != nullptr)
      *outMaterial = material;
    return true;
  }
  return false;
}
