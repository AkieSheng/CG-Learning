#include "sphere.h"
#include <cmath>
#include "grid.h"
#include "boundingbox.h"
#include "matrix.h"
#include "raytracing_stats.h"

Sphere::Sphere(Vec3f center, float radius, Material *m)
    : center(center), radius(radius)
{
  material = m;
  Vec3f rVec(radius, radius, radius);
  bbox = new BoundingBox(center - rVec, center + rVec);
}

void Sphere::insertIntoGrid(Grid *g, Matrix *m)
{
  if (g == nullptr)
    return;

  if (m != nullptr)
  {
    Object3D::insertIntoGrid(g, m);
    return;
  }

  float testRadius = radius + g->getVoxelHalfDiagonal();

  Vec3f rmin = bbox->getMin();
  Vec3f rmax = bbox->getMax();

  Vec3f bbMin = g->getBoundingBox()->getMin();
  Vec3f bbMax = g->getBoundingBox()->getMax();
  float dx = (bbMax.x() - bbMin.x()) / g->getNX();
  float dy = (bbMax.y() - bbMin.y()) / g->getNY();
  float dz = (bbMax.z() - bbMin.z()) / g->getNZ();
  int i0 = static_cast<int>((rmin.x() - bbMin.x()) / dx);
  int i1 = static_cast<int>((rmax.x() - bbMin.x()) / dx);
  int j0 = static_cast<int>((rmin.y() - bbMin.y()) / dy);
  int j1 = static_cast<int>((rmax.y() - bbMin.y()) / dy);
  int k0 = static_cast<int>((rmin.z() - bbMin.z()) / dz);
  int k1 = static_cast<int>((rmax.z() - bbMin.z()) / dz);
  if (i0 < 0) i0 = 0;
  if (j0 < 0) j0 = 0;
  if (k0 < 0) k0 = 0;
  if (i1 >= g->getNX()) i1 = g->getNX() - 1;
  if (j1 >= g->getNY()) j1 = g->getNY() - 1;
  if (k1 >= g->getNZ()) k1 = g->getNZ() - 1;

  Object3D *stored = g->wrapForGrid(this, nullptr);

  for (int i = i0; i <= i1; i++) {
    for (int j = j0; j <= j1; j++) {
      for (int k = k0; k <= k1; k++) {
        Vec3f voxelCenter = g->getVoxelCenter(i, j, k);
        if ((voxelCenter - center).Length() <= testRadius)
          g->insertObject(i, j, k, stored);
      }
    }
  }
}

bool Sphere::intersect(Ray const&r, Hit &h, float tmin)
{
  RayTracingStats::IncrementNumIntersections();
  Vec3f oc = r.getOrigin() - center;
  Vec3f dir = r.getDirection();
  float a = dir.Dot3(dir);
  float b = 2.0f * oc.Dot3(dir);
  float c = oc.Dot3(oc) - radius * radius;
  float discriminant = b * b - 4.0f * a * c;
  if (discriminant < 0.0f)
    return false;

  float sqrt_disc = ::sqrtf(discriminant);
  bool hit = false;
  float t = (-b - sqrt_disc) / (2.0f * a);
  if (t >= tmin && t < h.getT()) {

    Vec3f normal = r.pointAtParameter(t) - center;
    normal.Normalize();
    h.set(t, material, normal, r);
    hit = true;
  }
  t = (-b + sqrt_disc) / (2.0f * a);
  if (t >= tmin && t < h.getT()) {
    Vec3f normal = r.pointAtParameter(t) - center;
    normal.Normalize();
    h.set(t, material, normal, r);
    hit = true;
  }
  return hit;
}

bool Sphere::intersectShadow(Ray const&r, float tmin, float tmax, float &t,
                             Material **outMaterial)
{
  Vec3f oc = r.getOrigin() - center;
  Vec3f dir = r.getDirection();
  float a = dir.Dot3(dir);
  float b = 2.0f * oc.Dot3(dir);
  float c = oc.Dot3(oc) - radius * radius;
  float discriminant = b * b - 4.0f * a * c;
  if (discriminant < 0.0f)
    return false;

  float sqrt_disc = ::sqrtf(discriminant);
  float tNear = (-b - sqrt_disc) / (2.0f * a);
  float tFar = (-b + sqrt_disc) / (2.0f * a);

  if (outMaterial == nullptr)
  {
    if ((tNear >= tmin && tNear <= tmax) || (tFar >= tmin && tFar <= tmax)) {
      t = (tNear >= tmin && tNear <= tmax) ? tNear : tFar;
      return true;
    }
    return false;
  }

  bool hit = false;
  float bestT = tmax;
  if (tNear >= tmin && tNear < bestT)
  {
    bestT = tNear;
    hit = true;
  }
  if (tFar >= tmin && tFar < bestT)
  {
    bestT = tFar;
    hit = true;
  }
  if (hit)
  {
    t = bestT;
    *outMaterial = material;
  }
  return hit;
}
