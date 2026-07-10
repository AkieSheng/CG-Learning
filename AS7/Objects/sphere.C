#include "sphere.h"
#include "grid.h"
#include "boundingbox.h"
#include "matrix.h"
#include "raytracing_stats.h"
#include <math.h>

Sphere::Sphere(Vec3f center, float radius, Material *m)
    : center(center), radius(radius) {
  material = m;
  Vec3f rVec(radius, radius, radius);
  bbox = new BoundingBox(center - rVec, center + rVec);  // 包围盒
}

// 将球体栅格化到体素网格
void Sphere::insertIntoGrid(Grid *g, Matrix *m) {
  if (g == NULL)
    return;

  // 有变换时用 AABB 插入
  if (m != NULL) {
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
  int i0 = (int)((rmin.x() - bbMin.x()) / dx);
  int i1 = (int)((rmax.x() - bbMin.x()) / dx);
  int j0 = (int)((rmin.y() - bbMin.y()) / dy);
  int j1 = (int)((rmax.y() - bbMin.y()) / dy);
  int k0 = (int)((rmin.z() - bbMin.z()) / dz);
  int k1 = (int)((rmax.z() - bbMin.z()) / dz);
  if (i0 < 0) i0 = 0;
  if (j0 < 0) j0 = 0;
  if (k0 < 0) k0 = 0;
  if (i1 >= g->getNX()) i1 = g->getNX() - 1;
  if (j1 >= g->getNY()) j1 = g->getNY() - 1;
  if (k1 >= g->getNZ()) k1 = g->getNZ() - 1;

  Object3D *stored = g->wrapForGrid(this, NULL);

  // 遍历体素网格，判断球体是否与体素相交
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

// 射线-球体求交
// 数学上：|O + tD - C|^2 = r^2 → 二次方程 a*t^2 + b*t + c = 0
bool Sphere::intersect(const Ray &r, Hit &h, float tmin) {
  RayTracingStats::IncrementNumIntersections();
  Vec3f oc = r.getOrigin() - center;  // 射线起点到球心的向量
  Vec3f dir = r.getDirection();
  float a = dir.Dot3(dir);  // D·D
  float b = 2.0f * oc.Dot3(dir);  // 2 * (O-C)·D
  float c = oc.Dot3(oc) - radius * radius;  // (O-C)·(O-C) - r^2
  float discriminant = b * b - 4.0f * a * c;  // b^2 - 4ac
  if (discriminant < 0.0f)
    return false;  // 判别式 < 0，无实根，不相交

  float sqrt_disc = sqrtf(discriminant);
  bool hit = false;
  float t = (-b - sqrt_disc) / (2.0f * a);  // 近端交点
  if (t >= tmin && t < h.getT()) {
    // 法线：n = (P - C) / |P - C|
    Vec3f normal = r.pointAtParameter(t) - center;
    normal.Normalize();
    h.set(t, material, normal, r);
    hit = true;  // 有交点
  }
  t = (-b + sqrt_disc) / (2.0f * a);  // 远端交点
  if (t >= tmin && t < h.getT()) {
    Vec3f normal = r.pointAtParameter(t) - center;
    normal.Normalize();
    h.set(t, material, normal, r);
    hit = true;  // 有交点
  }
  return hit;
}

// 阴影射线求交
bool Sphere::intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                             Material **outMaterial) {
  Vec3f oc = r.getOrigin() - center;  // 射线起点到球心的向量
  Vec3f dir = r.getDirection();
  float a = dir.Dot3(dir);  // D·D
  float b = 2.0f * oc.Dot3(dir);  // 2 * (O-C)·D
  float c = oc.Dot3(oc) - radius * radius;  // (O-C)·(O-C) - r^2
  float discriminant = b * b - 4.0f * a * c;  // b^2 - 4ac
  if (discriminant < 0.0f)
    return false;  // 无实根，不相交

  float sqrt_disc = sqrtf(discriminant);
  float tNear = (-b - sqrt_disc) / (2.0f * a);  // 近端交点
  float tFar = (-b + sqrt_disc) / (2.0f * a);  // 远端交点

  if (outMaterial == NULL) {
    if ((tNear >= tmin && tNear <= tmax) || (tFar >= tmin && tFar <= tmax)) {
      t = (tNear >= tmin && tNear <= tmax) ? tNear : tFar;  // 选择最近的交点
      return true;
    }
    return false;  // 无交点
  }

  // 寻找最近的交点并返回材质
  bool hit = false;
  float bestT = tmax;
  if (tNear >= tmin && tNear < bestT) {
    bestT = tNear;
    hit = true;
  }
  if (tFar >= tmin && tFar < bestT) {
    bestT = tFar;
    hit = true;
  }
  if (hit) {
    t = bestT;
    *outMaterial = material;
  }
  return hit;
}
